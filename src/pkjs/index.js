// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

Pebble.addEventListener('showConfiguration', function(e) { Pebble.openURL(clay.generateUrl()); });

Pebble.addEventListener('webviewclosed',
    function(e) {
        if (e && !e.response) {
          return;
        }
      
        // Get the keys and values from each config item
        var dict = clay.getSettings(e.response);
      
        // Send settings values to watch side
        Pebble.sendAppMessage(dict, function(e) {
          console.log('Sent config data to Pebble');
        }, function(e) {
          console.log('Failed to send config data!');
          console.log(JSON.stringify(e));
        });

        // Grab OWM API token and save it to JS storage
        dict = clay.getSettings(e.response, false);
        if (dict.WeatherToken) {
                console.log("Weathertoken: " + dict.WeatherToken.value);
                localStorage.setItem("WeatherToken", dict.WeatherToken.value);
        }

        // Grab weather units pref and save it to JS storage
        if (dict.PrefWeatherUnits) {
                console.log("PrefWeatherUnits: " + dict.PrefWeatherUnits.value);
                localStorage.setItem("PrefWeatherUnits", dict.PrefWeatherUnits.value);
        }
    }
);


// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    if (localStorage.getItem("WeatherToken")) {
        console.log('Stored OWM token: ' + localStorage.getItem("WeatherToken"));
    } else {
        console.log('No OWM token stored');
    }
    if (localStorage.getItem("PrefWeatherUnits")) {
        console.log('Stored weather units pref: ' + localStorage.getItem("PrefWeatherUnits"));
    } else {
        console.log('No weather units pref stored');
    }

    // Just for debugging, we're going to launch a weather request
    // right now. Remember to remove this when the watch side
    // gets its smarts about when and how to request data
    //getWeather();

    // Let watch know we're ready
    Pebble.sendAppMessage({'JSReady': 1});
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
        console.log('Got message: ' + JSON.stringify(dict));
        var dict = e.payload;
        if(dict['WeatherRequestPls']) {
                // Watch has asked us to get the weather
                getWeather();
        }
  }                     
);

function locationSuccess(pos) {
        // We will request the weather here
        var url = 'https://api.openweathermap.org/data/2.5/weather?lat=' + pos.coords.latitude
                + '&lon=' + pos.coords.longitude
                + '&appid=' + localStorage.getItem("WeatherToken");

        // Send request to OpenWeatherMap
        var xhrRequest = new XMLHttpRequest();
        xhrRequest.onload =  function() {
                // responseText contains a JSON object with weather info
                var json = JSON.parse(this.responseText);

                // console.log("API response: " + JSON.stringify(this.responseText));

                // Temperature in Kelvin requires adjustment (we will do this on the watch i think)
                var temperature = json.main.temp;
                // switch (localStorage.getItem("PrefWeatherUnits")) {
                //         case 'c' :
                //                 // Celsius
                //                 temperature = Math.round(temperature - 273.15);
                //                 console.log('using celsius units');
                //                 break;
                //         default:
                //                 // Fahrenheit
                //                 temperature = Math.round((temperature - 273.15) * 9/5 + 32);
                //                 console.log('using fahrenheit units');
                //                 break;
                        
                // }
                console.log('Temperature is ' + temperature);

                // Conditions
                var conditions = json.weather[0].main;      
                console.log('Conditions are ' + conditions);

                var dict = {
                        'WeatherReturnCode': 0, // where 0=OK
                        'WeatherTemperatureK': temperature,
                        'WeatherCondition': conditions
                };
        
                // Send the object
                Pebble.sendAppMessage(dict, function() {
                console.log('Message sent successfully: ' + JSON.stringify(dict));
                }, function(e) {
                console.log('Message failed: ' + JSON.stringify(e));
                });
        };
        xhrRequest.open('GET', url);
        xhrRequest.send();
}
      
function locationError(err) {
        console.log('Error requesting location: ' + err.message);
        // Send error to watch
        sendWeatherError(1); // where 1=Error
}

function getWeather() {
        
        if (!localStorage.getItem("WeatherToken")) {
                // User hasn't set a token, throw an error to the watch
                sendWeatherError(2); // where 2=NoToken
                return;
        }
        // We have a token saved, go ahead!
        navigator.geolocation.getCurrentPosition(
                locationSuccess,
                locationError,
                {timeout: 15000, maximumAge: 60000});   
}

function sendWeatherError(exitCode) {
        var dict = {
                'WeatherReturnCode': exitCode, // where 0=OK, 1=Error, and 2=NoToken
        };

        // Send the object
        Pebble.sendAppMessage(dict, function() {
        console.log('Message sent successfully: ' + JSON.stringify(dict));
        }, function(e) {
        console.log('Message failed: ' + JSON.stringify(e));
        });
}