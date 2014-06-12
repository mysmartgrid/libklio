#include <boost/uuid/uuid_io.hpp>

#include "store.hpp"


using namespace klio;

const SensorFactory::Ptr Store::sensor_factory(new SensorFactory());
const TimeConverter::Ptr Store::time_converter(new TimeConverter());

void Store::start_transaction() {

    check_auto_commit_disabled();
    _transaction->start();
}

void Store::commit_transaction() {

    check_auto_commit_disabled();
    _transaction->commit();
}

void Store::rollback_transaction() {

    check_auto_commit_disabled();
    _transaction->rollback();
    Store::prepare();
}

void Store::check_auto_commit_disabled() {

    if (_auto_commit) {
        std::ostringstream oss;
        oss << "This operation can not be performed because the store is configured to perform commits automatically.";
        throw StoreException(oss.str());
    }
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

void Store::start_inner_transaction(const Transaction::Ptr transaction) {
        
    if (_auto_commit) {
        transaction->start();

    } else if (!_transaction->pending()) {
        std::ostringstream oss;
        oss << "Automatic commits are disabled for this store. Please, start a transaction manually before invoking this method.";
        throw StoreException(oss.str());
    }
}

void Store::commit_inner_transaction(const Transaction::Ptr transaction) {

    if (_auto_commit) {
        transaction->commit();
    }
}

void Store::add_sensor(const Sensor::Ptr sensor) {

    LOG("Adding sensor: " << sensor->str());
    
    Transaction::Ptr transaction = get_transaction();
    start_inner_transaction(transaction);
        
    add_sensor_record(sensor);

    commit_inner_transaction(transaction);
    set_buffers(sensor);
}

void Store::remove_sensor(const Sensor::Ptr sensor) {

    LOG("Removing sensor: " << sensor->str());

    Transaction::Ptr transaction = get_transaction();
    start_inner_transaction(transaction);

    remove_sensor_record(sensor);

    commit_inner_transaction(transaction);
    clear_buffers(sensor);
}

void Store::update_sensor(const Sensor::Ptr sensor) {

    LOG("Updating sensor: " << sensor->str());

    Transaction::Ptr transaction = get_transaction();
    start_inner_transaction(transaction);

    update_sensor_record(sensor);

    commit_inner_transaction(transaction);
    set_buffers(sensor);
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

    return get_sensors_records();
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

    for (std::map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {

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

    for (std::map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {
        uuids.push_back((*it).first);
    }
    return uuids;
}

void Store::add_reading(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value) {

    LOG("Adding to sensor: " << sensor->str() << " time=" << timestamp << " value=" << value);

    //Check if sensor exists
    get_sensor(sensor->uuid());
    set_buffers(sensor);
    
    Transaction::Ptr transaction = get_transaction();
    start_inner_transaction(transaction);
    
    add_reading_record(sensor, timestamp, value);
    
    commit_inner_transaction(transaction);
    Store::flush(false);
}

void Store::add_readings(const Sensor::Ptr sensor, const readings_t& readings) {

    LOG("Adding " << readings->size() << " readings to sensor: " << sensor->str());

    //Check if sensor exists
    get_sensor(sensor->uuid());
    set_buffers(sensor);
    
    Transaction::Ptr transaction = get_transaction();
    start_inner_transaction(transaction);
    
    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        add_reading_record(sensor, (*it).first, (*it).second);
    }

    commit_inner_transaction(transaction);
    Store::flush(false);
}

void Store::update_readings(const Sensor::Ptr sensor, const readings_t& readings) {

    LOG("Updating " << readings->size() << " readings of sensor: " << sensor->str());

    //Check if sensor exists
    get_sensor(sensor->uuid());
    set_buffers(sensor);

    Transaction::Ptr transaction = get_transaction();
    start_inner_transaction(transaction);
    
    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        update_reading_record(sensor, (*it).first, (*it).second);
    }

    commit_inner_transaction(transaction);
    Store::flush(false);
}

void Store::add_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value) {

    _readings_buffer[sensor->uuid()]->insert(reading_t(timestamp, value));
}

void Store::update_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value) {

    _readings_buffer[sensor->uuid()]->insert(reading_t(timestamp, value));
}

readings_t_Ptr Store::get_all_readings(const Sensor::Ptr sensor) {

    LOG("Retrieving all readings of sensor " << sensor->str());

    //Check if sensor exists
    get_sensor(sensor->uuid());

    flush(sensor);

    return get_all_readings_records(sensor);
}

readings_t_Ptr Store::get_timeframe_readings(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end) {

    LOG("Retrieving readings of sensor " << sensor->str() << " between " << begin << " and " << end);

    //Check if sensor exists
    get_sensor(sensor->uuid());

    flush(sensor);

    return get_timeframe_readings_records(sensor, begin, end);
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

void Store::prepare() {

    clear_buffers();

    std::vector<Sensor::Ptr> sensors = get_sensors();
    for (std::vector<Sensor::Ptr>::const_iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {
        set_buffers(*sensor);
    }
}

void Store::flush() {
    flush(true);
}

void Store::flush(bool force) {

    timestamp_t now = time_converter->get_timestamp();

    if (force || now - _last_sync >= _sync_timeout) {

        for (std::map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {

            flush((*it).second);
        }
        _last_sync = now;
    }
}

void Store::set_buffers(const Sensor::Ptr sensor) {

    if (_external_ids_buffer.count(sensor->external_id()) > 0) {

        Sensor::uuid_t other_uuid = _external_ids_buffer[sensor->external_id()];
        _sensors_buffer.erase(other_uuid);

        if (_readings_buffer.count(other_uuid) > 0) {
            _readings_buffer[sensor->uuid()] = _readings_buffer[other_uuid];
            _readings_buffer.erase(other_uuid);
        }
    }

    if (_readings_buffer.count(sensor->uuid()) == 0) {
        _readings_buffer[sensor->uuid()] = readings_t_Ptr(new readings_t());
    }

    _sensors_buffer[sensor->uuid()] = sensor;
    _external_ids_buffer[sensor->external_id()] = sensor->uuid();
}

void Store::clear_buffers(const Sensor::Ptr sensor) {

    _sensors_buffer.erase(sensor->uuid());
    _readings_buffer.erase(sensor->uuid());
    _external_ids_buffer.erase(sensor->external_id());
}

void Store::clear_buffers() {

    _sensors_buffer.clear();
    _readings_buffer.clear();
    _external_ids_buffer.clear();
}
