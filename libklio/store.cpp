#include <boost/uuid/uuid_io.hpp>
#include <libklio/store.hpp>


using namespace klio;

const SensorFactory::Ptr Store::sensor_factory(new SensorFactory());
const TimeConverter::Ptr Store::time_converter(new TimeConverter());

const Store::cached_operation_type_t Store::INSERT_OPERATION = 1;
const Store::cached_operation_type_t Store::UPDATE_OPERATION = 2;
const Store::cached_operation_type_t Store::DELETE_OPERATION = 3;

void Store::start_transaction() {

    check_out_commit_off();
    get_transaction_handler()->start();
}

void Store::commit_transaction() {

    check_out_commit_off();
    flush_all(_auto_flush);
    get_transaction_handler()->commit();
}

void Store::rollback_transaction() {

    check_out_commit_off();
    get_transaction_handler()->rollback();
    clear_buffers();
    prepare();
}

void Store::check_out_commit_off() {

    if (_auto_commit) {
        std::ostringstream oss;
        oss << "This operation can not be performed because the store is configured to perform commits automatically.";
        throw StoreException(oss.str());
    }
}

Transaction::Ptr Store::get_transaction_handler() {

    //By default, stores do not provide transactions
    return Transaction::Null;
}

Transaction::Ptr Store::auto_start_transaction() {

    Transaction::Ptr transaction = get_transaction_handler();

    if (transaction) {

        if (_auto_commit) {
            transaction->start();

        } else if (!transaction->pending()) {
            std::ostringstream oss;
            oss << "Automatic commits are disabled for this store. Please, start a transaction manually before invoking this method.";
            throw StoreException(oss.str());
        }
    }
    return transaction;
}

void Store::auto_commit_transaction(const Transaction::Ptr transaction) {

    if (transaction && _auto_commit) {
        transaction->commit();
    }
}

void Store::add_sensor(const Sensor::Ptr sensor) {

    LOG("Adding sensor: " << sensor->str());

    const Transaction::Ptr transaction = auto_start_transaction();

    add_sensor_record(sensor);

    auto_commit_transaction(transaction);
    set_buffers(sensor);
}

void Store::remove_sensor(const Sensor::Ptr sensor) {

    LOG("Removing sensor: " << sensor->str());

    const Transaction::Ptr transaction = auto_start_transaction();

    remove_sensor_record(sensor);

    auto_commit_transaction(transaction);
    clear_buffers(sensor);
}

void Store::update_sensor(const Sensor::Ptr sensor) {

    LOG("Updating sensor: " << sensor->str());

    const Transaction::Ptr transaction = auto_start_transaction();

    update_sensor_record(sensor);

    auto_commit_transaction(transaction);
    set_buffers(sensor);
}

void Store::add_reading(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value) {

    LOG("Adding to sensor: " << sensor->str() << " time=" << timestamp << " value=" << value);

    klio::readings_t_Ptr cached_readings = get_buffered_readings(sensor, INSERT_OPERATION);
    cached_readings->insert(reading_t(timestamp, value));
    auto_flush();
}

void Store::add_readings(const Sensor::Ptr sensor, const readings_t& readings) {

    LOG("Adding " << readings->size() << " readings to sensor: " << sensor->str());

    add_readings(sensor, readings, INSERT_OPERATION);
}

void Store::update_readings(const Sensor::Ptr sensor, const readings_t& readings) {

    LOG("Updating " << readings->size() << " readings of sensor: " << sensor->str());

    add_readings(sensor, readings, UPDATE_OPERATION);
}

void Store::add_readings(const Sensor::Ptr sensor, const readings_t& readings, const cached_operation_type_t operation_type) {

    if (!readings.empty()) {

        klio::readings_t_Ptr cached_readings = get_buffered_readings(sensor, operation_type);
        for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
            cached_readings->insert(reading_t((*it).first, (*it).second));
        }
        auto_flush();
    }
}

readings_t_Ptr Store::get_buffered_readings(const Sensor::Ptr sensor, const cached_operation_type_t operation_type) {

    //Check if sensor exists
    get_sensor(sensor->uuid());

    cached_reading_operations_type_t_Ptr cached_operations = _reading_operations_buffer[sensor->uuid()];
    return cached_operations->at(operation_type);
}

Sensor::Ptr Store::get_sensor(const Sensor::uuid_t& uuid) {

    LOG("Getting sensor by UUID: " << uuid);

    boost::unordered_map<const Sensor::uuid_t, Sensor::Ptr>::const_iterator found = _sensors_buffer.find(uuid);

    if (found == _sensors_buffer.end()) {
        std::ostringstream err;
        err << "Sensor " << boost::uuids::to_string(uuid) << " could not be found.";
        throw StoreException(err.str());
    } else {
        return found->second;
    }
}

std::vector<Sensor::Ptr> Store::get_sensors() {

    LOG("Getting all sensors");

    return get_sensor_records();
}

std::vector<Sensor::Ptr> Store::get_sensors_by_external_id(const std::string& external_id) {

    LOG("Getting sensors by external id");

    std::vector<Sensor::Ptr> sensors;

    if (_external_ids_buffer.count(external_id) > 0) {
        Sensor::uuid_t uuid = _external_ids_buffer[external_id];
        sensors.push_back(_sensors_buffer[uuid]);
    }
    return sensors;
}

std::vector<Sensor::Ptr> Store::get_sensors_by_name(const std::string& name) {

    LOG("Getting sensors by name");

    std::vector<Sensor::Ptr> sensors;

    for (boost::unordered_map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {

        Sensor::Ptr sensor = (*it).second;

        if (sensor->name() == name) {
            sensors.push_back(sensor);
        }
    }
    return sensors;
}

std::vector<Sensor::uuid_t> Store::get_sensor_uuids() {

    LOG("Getting sensor UUIDs");

    std::vector<Sensor::uuid_t> uuids;

    for (boost::unordered_map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {
        uuids.push_back((*it).first);
    }
    return uuids;
}

readings_t_Ptr Store::get_all_readings(const Sensor::Ptr sensor) {

    LOG("Retrieving all readings of sensor " << sensor->str());

    flush(sensor, true);

    return get_all_reading_records(sensor);
}

readings_t_Ptr Store::get_timeframe_readings(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end) {

    LOG("Retrieving readings of sensor " << sensor->str() << " between " << begin << " and " << end);

    flush(sensor, true);

    return get_timeframe_reading_records(sensor, begin, end);
}

unsigned long int Store::get_num_readings(const Sensor::Ptr sensor) {

    LOG("Retrieving number of readings for sensor " << sensor->str());

    flush(sensor, true);

    return get_num_readings_value(sensor);
}

reading_t Store::get_last_reading(const Sensor::Ptr sensor) {

    LOG("Retrieving last reading of sensor " << sensor->str());

    flush(sensor, true);

    return get_last_reading_record(sensor);
}

reading_t Store::get_reading(const Sensor::Ptr sensor, const timestamp_t timestamp) {

    LOG("Retrieving reading of sensor " << sensor->str());

    flush(sensor, true);

    return get_reading_record(sensor, timestamp);
}

void Store::sync(const Store::Ptr store) {

    LOG("Synchronizing this store with store " << store->str());

    const std::vector<Sensor::Ptr> sensors = store->get_sensors();
    const Transaction::Ptr transaction = auto_start_transaction();

    for (std::vector<Sensor::Ptr>::const_iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {
        sync_reading_records(*sensor, store);
    }
    auto_commit_transaction(transaction);
}

void Store::sync_readings(const Sensor::Ptr sensor, const Store::Ptr store) {

    LOG("Synchronizing this store readings with the readings of sensor " << sensor->str() << " from store " << store->str());

    const Transaction::Ptr transaction = auto_start_transaction();
    sync_reading_records(sensor, store);
    auto_commit_transaction(transaction);
}

void Store::sync_sensors(const Store::Ptr store) {

    LOG("Synchronizing this store sensors with the sensors of store " << store->str());

    const std::vector<Sensor::Ptr> sensors = store->get_sensors();
    const Transaction::Ptr transaction = auto_start_transaction();

    for (std::vector<Sensor::Ptr>::const_iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {

        set_buffers(sync_sensor_record(*sensor));
    }
    auto_commit_transaction(transaction);
}

void Store::sync_reading_records(const Sensor::Ptr sensor, const Store::Ptr store) {

    readings_t_Ptr readings = store->get_all_readings(sensor);
    Sensor::Ptr local_sensor = sync_sensor_record(sensor);
    set_buffers(local_sensor);
    add_readings(local_sensor, *readings, UPDATE_OPERATION);
}

Sensor::Ptr Store::sync_sensor_record(const Sensor::Ptr sensor) {

    Sensor::Ptr local_sensor;

    if (_external_ids_buffer.count(sensor->external_id()) > 0) {

        const Sensor::uuid_t uuid = _external_ids_buffer[sensor->external_id()];
        const Sensor::Ptr found = _sensors_buffer[uuid];

        local_sensor = sensor_factory->createSensor(
                uuid,
                sensor->external_id(),
                sensor->name(),
                sensor->description(),
                sensor->unit(),
                sensor->timezone(),
                sensor->device_type());

        //Update sensor if any of its properties changed
        if (found->name() != sensor->name() ||
                found->description() != sensor->description() ||
                found->unit() != sensor->unit() ||
                found->timezone() != sensor->timezone() ||
                found->device_type() != sensor->device_type()) {

            update_sensor_record(local_sensor);
        }

    } else {
        add_sensor_record(sensor);
        local_sensor = sensor;
    }
    return local_sensor;
}

void Store::prepare() {

    std::vector<Sensor::Ptr> sensors = get_sensor_records();
    for (std::vector<Sensor::Ptr>::const_iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {
        set_buffers(*sensor);
    }
}

void Store::flush() {

    flush(_auto_flush);
}

void Store::flush(bool ignore_errors) {

    const Transaction::Ptr transaction = auto_start_transaction();
    flush_all(ignore_errors);
    auto_commit_transaction(transaction);
}

void Store::auto_flush() {

    const timestamp_t now = time_converter->get_timestamp();

    if (_auto_flush && now - _last_flush >= _flush_timeout) {

        const Transaction::Ptr transaction = auto_start_transaction();
        flush_all(true);
        auto_commit_transaction(transaction);
        _last_flush = now;
    }
}

void Store::flush_all(bool ignore_errors) {

    for (boost::unordered_map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {
        flush((*it).second, ignore_errors);
    }
}

void Store::flush(const Sensor::Ptr sensor, bool ignore_errors) {

    boost::unordered_map<const Sensor::uuid_t, cached_reading_operations_type_t_Ptr>::const_iterator found =
            _reading_operations_buffer.find(sensor->uuid());

    if (found == _reading_operations_buffer.end()) {
        std::ostringstream err;
        err << "Sensor " << sensor->uuid_string() << " could not be found.";
        throw StoreException(err.str());
    }

    readings_t_Ptr readings = found->second->at(INSERT_OPERATION);

    //Small number of insertions
    if (readings->size() <= _min_bulk_size) {

        for (readings_cit_t it = readings->begin(); it != readings->end(); ++it) {
            add_single_reading_record(sensor, (*it).first, (*it).second, ignore_errors);
        }

        //Bulk insertion
    } else {

        readings_cit_t reading = readings->begin();
        readings_t_Ptr bulk(new readings_t());
        int bulks = (int) ceil(((double) readings->size()) / ((double) _max_bulk_size));

        while (bulks-- > 0) {

            while (reading != readings->end() && bulk->size() < _max_bulk_size) {
                bulk->insert(*reading);
                reading++;
            }
            add_bulk_reading_records(sensor, *bulk, ignore_errors);
            bulk->clear();
        }
    }
    readings->clear();

    readings = found->second->at(UPDATE_OPERATION);
    if (readings->size() > 0) {
        update_reading_records(sensor, *readings, ignore_errors);
        readings->clear();
    }
}

void Store::set_buffers(const Sensor::Ptr sensor) {

    if (_external_ids_buffer.count(sensor->external_id()) > 0) {

        Sensor::uuid_t other_uuid = _external_ids_buffer[sensor->external_id()];
        _sensors_buffer.erase(other_uuid);

        if (_reading_operations_buffer.count(other_uuid) > 0) {
            _reading_operations_buffer[sensor->uuid()] = _reading_operations_buffer[other_uuid];
            _reading_operations_buffer.erase(other_uuid);
        }
    }

    if (_reading_operations_buffer.count(sensor->uuid()) == 0) {

        cached_reading_operations_type_t_Ptr cached_operations = cached_reading_operations_type_t_Ptr(new cached_reading_operations_type_t());
        cached_operations->insert(cached_readings_type_t(INSERT_OPERATION, readings_t_Ptr(new readings_t())));
        cached_operations->insert(cached_readings_type_t(UPDATE_OPERATION, readings_t_Ptr(new readings_t())));
        _reading_operations_buffer[sensor->uuid()] = cached_operations;
    }

    _sensors_buffer[sensor->uuid()] = sensor;
    _external_ids_buffer[sensor->external_id()] = sensor->uuid();
}

void Store::clear_buffers(const Sensor::Ptr sensor) {

    _sensors_buffer.erase(sensor->uuid());
    _external_ids_buffer.erase(sensor->external_id());
    _reading_operations_buffer.erase(sensor->uuid());
}

void Store::clear_buffers() {

    _sensors_buffer.clear();
    _external_ids_buffer.clear();
    _reading_operations_buffer.clear();
}

void Store::handle_reading_insertion_error(const bool ignore_errors, const timestamp_t timestamp, const double value) {

    std::ostringstream oss;
    oss << "Error adding reading: (" << timestamp << ", " << value << ")";
    handle_reading_insertion_error(ignore_errors, oss.str());
}

void Store::handle_reading_insertion_error(const bool ignore_errors, const Sensor::Ptr sensor) {

    std::ostringstream oss;
    oss << "Error adding readings for sensor: " << sensor->uuid_string();
    handle_reading_insertion_error(ignore_errors, oss.str());
}

void Store::handle_reading_insertion_error(const bool ignore_errors, const std::string message) {

    if (ignore_errors) {
        LOG(message);
    } else {
        throw StoreException(message);
    }
}