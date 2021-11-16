const char index_page[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://code.highcharts.com/highcharts.js"></script>
  <title>GROWATT</title>
  <style>
    body {
      min-width: 310px;
    	max-width: 800px;
    	height: 400px;
      margin: 0 auto;
    }
    h2 {
      font-family: Arial;
      font-size: 2.5rem;
      text-align: center;
    }
    table {
      width: 100%;
    }
  </style>
</head>
<body>
  <h2>Growatt</h2>
  <div id="chart-power" class="container"></div>
  <div>
    <table>
      <tr>
        <td>Power:</td>
        <td id="TD_ACPower"></td>
      </tr>
      <tr>
        <td>Today:</td>
        <td id="TD_EnergyToday"></td>
      </tr>
      <tr>
        <td>Total:</td>
        <td id="TD_EnergyTotal"></td>
      </tr>
       <tr>
        <td>DC Voltage:</td>
        <td id="TD_DCVoltage"></td>
      </tr>
    </table>
  </div>
  <br> 
  <a href="./firmware">Firmware update</a><br>
  <a href="./status">JSON</a><br>
  <a href="./settings">Settings</a><br>
</body>
<script>
var chartT = new Highcharts.Chart({
  chart:{ renderTo : 'chart-power' },
  title: { text: '' },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    },
    series: { color: '#0000FF' }
  },
  xAxis: { type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { text: 'Power [W]' }
  },
  credits: { enabled: false }
});

var count;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function(){
    if (this.readyState == 4 && this.status == 200){
      var obj = JSON.parse(this.responseText);
      if (count != obj.count){
        document.getElementById("TD_ACPower").innerHTML = obj.ac_power + " W";
        document.getElementById("TD_EnergyToday").innerHTML = obj.energy_today + " kWh"; 
        document.getElementById("TD_EnergyTotal").innerHTML = obj.energy_total + " kWh"; 
        document.getElementById("TD_DCVoltage").innerHTML = obj.dc_voltage + " V"; 
  
        var t = new Date().getTime()+(-new Date().getTimezoneOffset()*60*1000);   
        var x = t;
        var y = obj.ac_power;
        console.log(this.responseText);
        if (chartT.series[0].data.length > 40) {
          chartT.series[0].addPoint([x, y], true, true, true);
        } else {
          chartT.series[0].addPoint([x, y], true, false, true);
        }
        count = obj.count;
      }
    }
  };
  xhttp.open("GET", "./status", true);
  xhttp.send();
}, 10000 ) ;


</script>
</html>
)=====";
