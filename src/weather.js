// Called when the watchface is opened
Pebble.addEventListener('ready',
	function(e) {
		console.log("Getting weather");
		getWeather();
	});

// Called when an AppMessage is received
Pebble.addEventListener('appmessage',
	function(e) {
		console.log("AppMessage received!");		
	});

var xhrRequest = function(url, type, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function() {
		callback(this.responseText);
	};
	xhr.open(type, url);
	xhr.send();
};

// Request weather for given location
function locationSuccess(pos) {
	var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude;
	
	xhrRequest(url, 'GET',
		function(responseText) {
			// the payload is JSON
			var json = JSON.parse(responseText);
			// convert from K
			var temperature = Math.round(json.main.temp - 273.15);
			var conditions = json.weather[0].main;
			
			console.log("Temp is " + temperature + "; conditions are " + conditions);
		});
}

function locationError(err) {
	console.log("Error requesting location");
}

function getWeather() {
	navigator.geolocation.getCurrentPosition(
		locationSuccess,
		locationError,
		{timeout: 15000, maximumAge: 60000}	);
}