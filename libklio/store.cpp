#include <boost/uuid/uuid_io.hpp>

#include "store.hpp"


using namespace klio;

const SensorFactory::Ptr Store::sensor_factory(new SensorFactory());
const TimeConverter::Ptr Store::time_converter(new TimeConverter());

const Store::cached_operation_type_t Store::INSERT_OPERATION = 1;
const Store::cached_operation_type_t Store::UPDATE_OPERATION = 2;
const Store::cached_operation_type_t Store::DELETE_OPERATION = 3;

void Store::open() {
    _transaction = create_transaction();
}

void Store::close() {

    if (!_auto_commit) {
        rollback_transaction();
    }
    clear_buffers();
}

void Store::dispose() {

    clear_buffers();
}

void Store::start_transaction() {

    if (_auto_commit) {
        std::ostringstream oss;
        oss << "This operation can not be performed because the store is configured to perform commits automatically.";
        throw StoreException(oss.str());
    }
    _transaction->start();
}

void Store::commit_transaction() {

    flush(true);
    _transaction->commit();
}

void Store::rollback_transaction() {

    _transaction->rollback();
    prepare();
}

Transaction::Ptr Store::get_transaction() {

    if (_auto_commit) {
        return create_transaction();
    } else {
        return _transaction;
    }
}

Transaction::Ptr Store::create_transaction() {

    //Default
    return Transaction::Ptr(new Transaction());
}

Transaction::Ptr Store::auto_start_transaction() {

    const Transaction::Ptr transaction = get_transaction();

    if (_auto_commit) {
        transaction->start();

    } else if (!_transaction->pending()) {
        std::ostringstream oss;
        oss << "Automatic commits are disabled for this store. Please, start a transaction manually before invoking this method.";
        throw StoreException(oss.str());
    }
    return transaction;
}

void Store::auto_commit_transaction(const Transaction::Ptr transaction) {

    if (_auto_commit) {
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

    cached_operations_type_t_Ptr cached_operations = _reading_operations_buffer[sensor->uuid()];
    return cached_operations->at(operation_type);
}

Sensor::Ptr Store::get_sensor(const Sensor::uuid_t& uuid) {

    LOG("Getting sensor by UUID: " << uuid);

    if (_sensors_buffer.count(uuid) > 0) {
        return _sensors_buffer[uuid];

    } else {
        std::ostringstream err;
        err << "Sensor " << boost::uuids::to_string(uuid) << " could not be found.";
        throw StoreException(err.str());
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

    //Check if sensor exists
    get_sensor(sensor->uuid());

    flush(sensor);

    return get_all_reading_records(sensor);
}

readings_t_Ptr Store::get_timeframe_readings(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end) {

    LOG("Retrieving readings of sensor " << sensor->str() << " between " << begin << " and " << end);

    //Check if sensor exists
    get_sensor(sensor->uuid());

    flush(sensor);

    return get_timeframe_reading_records(sensor, begin, end);
}

unsigned long int Store::get_num_readings(const Sensor::Ptr sensor) {

    LOG("Retrieving number of readings for sensor " << sensor->str());

    //Check if sensor exists
    get_sensor(sensor->uuid());

    flush(sensor);

    return get_num_readings_value(sensor);
}

reading_t Store::get_last_reading(const Sensor::Ptr sensor) {

    LOG("Retrieving last reading of sensor " << sensor->str());

    //Check if sensor exists
    get_sensor(sensor->uuid());

    flush(sensor);

    return get_last_reading_record(sensor);
}

reading_t Store::get_reading(const Sensor::Ptr sensor, const timestamp_t timestamp) {

    LOG("Retrieving reading of sensor " << sensor->str());

    //Check if sensor exists
    get_sensor(sensor->uuid());

    flush(sensor);

    return get_reading_record(sensor, timestamp);
}

void Store::sync(const Store::Ptr store) {

    LOG("Synchronizing this store with store " << store->str());

    const std::vector<Sensor::Ptr> sensors = store->get_sensors();

    for (std::vector<Sensor::Ptr>::const_iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {

        sync_readings(*sensor, store);
    }
}

void Store::sync_readings(const Sensor::Ptr sensor, const Store::Ptr store) {

    LOG("Synchronizing this store readings with the readings of sensor " << sensor->str() << " from store " << store->str());

    klio::readings_t_Ptr readings = store->get_all_readings(sensor);
    std::vector<Sensor::Ptr> sensors = get_sensors_by_external_id(sensor->external_id());
    Sensor::Ptr local_sensor;

    if (sensors.empty()) {

        add_sensor(sensor);
        local_sensor = sensor;

    } else {
        //Exactly one sensor is processed
        for (std::vector<Sensor::Ptr>::const_iterator found = sensors.begin(); found != sensors.end(); ++found) {

            local_sensor = sensor_factory->createSensor(
                    (*found)->uuid(),
                    sensor->external_id(),
                    sensor->name(),
                    sensor->description(),
                    sensor->unit(),
                    sensor->timezone(),
                    sensor->device_type());

            //Update sensor if any of its properties changed
            if ((*found)->name() != sensor->name() ||
                    (*found)->description() != sensor->description() ||
                    (*found)->unit() != sensor->unit() ||
                    (*found)->timezone() != sensor->timezone() ||
                    (*found)->device_type() != sensor->device_type()) {

                update_sensor(local_sensor);
            }
        }
    }

    update_readings(local_sensor, *readings);
}

void Store::sync_sensors(const Store::Ptr store) {

    LOG("Synchronizing this store sensors with the sensors of store " << store->str());

    const std::vector<Sensor::Ptr> sensors = store->get_sensors();

    const Transaction::Ptr transaction = auto_start_transaction();

    for (std::vector<Sensor::Ptr>::const_iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {
        add_sensor_record(*sensor);
        set_buffers(*sensor);
    }
    auto_commit_transaction(transaction);
}

void Store::prepare() {

    clear_buffers();

    std::vector<Sensor::Ptr> sensors = get_sensors();
    for (std::vector<Sensor::Ptr>::const_iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {
        set_buffers(*sensor);
    }
}

void Store::auto_flush() {

    if (_auto_flush) {
        const Transaction::Ptr transaction = auto_start_transaction();
        flush(false);
        auto_commit_transaction(transaction);
    }
}

void Store::flush() {
    flush(true);
}

void Store::flush(const bool force) {

    const timestamp_t now = time_converter->get_timestamp();

    if (force || now - _last_flush >= _flush_timeout) {

        for (boost::unordered_map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {

            flush((*it).second);
        }
        _last_flush = now;
    }
}

void Store::flush(const Sensor::Ptr sensor) {

    cached_operations_type_t_Ptr operations = _reading_operations_buffer[sensor->uuid()];
    readings_t_Ptr readings = operations->at(INSERT_OPERATION);
    if (readings->size() > 0) {
        add_reading_records(sensor, *readings);
        readings->clear();
    }

    readings = operations->at(UPDATE_OPERATION);
    if (readings->size() > 0) {
        update_reading_records(sensor, *readings);
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

        cached_operations_type_t_Ptr cached_operations = cached_operations_type_t_Ptr(new cached_operations_type_t());
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
