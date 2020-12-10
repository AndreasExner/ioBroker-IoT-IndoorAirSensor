on({id: "0_userdata.0.IoT-Devices.04.SensorIP", change: "any"}, function (obj) {
  var value = obj.state.val;
  var oldValue = obj.oldState.val;
  setState("0_userdata.0.IoT.IndoorAirSensor.LastUpdate", ([formatDate(new Date(), "DD.MM."),' - ',formatDate(new Date(), "hh:mm")].join('')), true);
});
