void sendMQTT_Power_DiscoveryMsg() {

  String discoveryTopic = "homeassistant/sensor/growatt_inverter_" + String(sensorNumber) + "/power/config";

  String payload = "{\"device\":{\"identifiers\":[\"growatt_inverter_" + String(sensorNumber) + "\"],\"manufacturer\":\"GroWatt\",\"name\":\"GroWatt Inverter\"},"
                   "\"device_class\":\"power\","
                   "\"force_update\":true,"
                   "\"name\":\"GroWatt Inverter Power\","
                   "\"state_class\":\"measurement\","
                   "\"state_topic\":\"" + stateTopic + "\","
                   "\"unique_id\":\"growatt_inverter_" + String(sensorNumber) + "_power\","
                   "\"unit_of_measurement\":\"W\","
                   "\"value_template\":\"{{ value_json.ac_power }}\","
                   "\"platform\":\"mqtt\"}";

  mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);
}

void sendMQTT_EnergyToday_DiscoveryMsg() {

  String discoveryTopic = "homeassistant/sensor/growatt_inverter_" + String(sensorNumber) + "/energy_today/config";

  String payload = "{\"device\":{\"identifiers\":[\"growatt_inverter_" + String(sensorNumber) + "\"],\"manufacturer\":\"GroWatt\",\"name\":\"GroWatt Inverter\"},"
                   "\"device_class\":\"energy\","
                   "\"force_update\":false,"
                   "\"name\":\"GroWatt Inverter Today Energy\","
                   "\"state_class\":\"total\","
                   "\"state_topic\":\"" + stateTopic + "\","
                   "\"unique_id\":\"growatt_inverter_" + String(sensorNumber) + "_energy_today\","
                   "\"unit_of_measurement\":\"kWh\","
                   "\"value_template\":\"{{ value_json.energy_today }}\","
                   "\"platform\":\"mqtt\"}";

  mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);
}

void sendMQTT_EnergyTotal_DiscoveryMsg() {

  String discoveryTopic = "homeassistant/sensor/growatt_inverter_" + String(sensorNumber) + "/energy/config";

  String payload = "{\"device\":{\"identifiers\":[\"growatt_inverter_" + String(sensorNumber) + "\"],\"manufacturer\":\"GroWatt\",\"name\":\"GroWatt Inverter\"},"
                   "\"device_class\":\"energy\","
                   "\"force_update\":false,"
                   "\"name\":\"GroWatt Inverter Total Energy\","
                   "\"state_class\":\"total_increasing\","
                   "\"state_topic\":\"" + stateTopic + "\","
                   "\"unique_id\":\"growatt_inverter_" + String(sensorNumber) + "_energy\","
                   "\"unit_of_measurement\":\"kWh\","
                   "\"value_template\":\"{{ value_json.energy_total }}\","
                   "\"platform\":\"mqtt\"}";

  mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);
}
