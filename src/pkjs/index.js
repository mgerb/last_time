const suncalc = require("./suncalc");

// Import the Clay package
var Clay = require("@rebble/clay");
// Load our Clay configuration file
var clayConfig = require("./config");
// Initialize Clay
var clay = new Clay(clayConfig);

const ERRORS = {
  unknown: 0,
  http: 1,
  json_parse: 2,
  location: 3
};

// Values mostly grabbed from here: https://github.com/open-meteo/open-meteo/issues/789
function weatherCodeToText(code) {
  switch (code) {
    case 0:
      return "Clear Sky";
    case 1:
      return "Mainly Clear";
    case 2:
      return "Partly Cloudy";
    case 3:
      return "Overcast";
    case 45:
      return "Fog";
    case 48:
      return "Icy Fog";
    case 51:
      return "Light Drizzle";
    case 53:
      return "Drizzle";
    case 55:
      return "Heavy Drizzle";
    case 56:
    case 57:
      return "Freezing Drizzle";
    case 61:
      return "Light Rain";
    case 63:
      return "Rain";
    case 65:
      return "Heavy Rain";
    case 66:
    case 67:
      return "Freezing Rain";
    case 71:
      return "Slight Snow";
    case 73:
      return "Moderate Snow";
    case 75:
      return "Heavy Snow";
    case 77:
      return "Snow Grains";
    case 80:
      return "Light Showers";
    case 81:
      return "Rain Showers";
    case 82:
      return "Heavy Showers";
    case 85:
      return "Light Snow Showers";
    case 86:
      return "Snow Showers";
    case 95:
      return "Thunderstorm";
    case 96:
    case 97:
      return "Storm w/ Hail";
    case 98:
    case 99:
      return "Severe Hail Storm";
    default:
      return "Unknown";
  }
}

function notifyWatchOfError(code) {
  Pebble.sendAppMessage(
    { error: code || ERRORS.unknown },
    function (e) {
      console.log("Sent error to watch", e);
    },
    function (e) {
      console.log("Failed to send error to watch", e);
    }
  );
}

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    var err = null;

    if (this.status >= 400) {
      err = "http error: " + this.status;
      console.log("http error:", err, this.responseText);
    }
    callback(err, this.responseText);
  };
  xhr.onerror = function (e) {
    console.log("network error:", e);
    callback("network error", null);
  };
  xhr.open(type, url);
  xhr.send();
};

function getWeatherFromLocation(lat, lon, callback) {
  var unit = "fahrenheit";
  var url =
    "https://api.open-meteo.com/v1/forecast?latitude=" +
    lat +
    "&longitude=" +
    lon +
    "&daily=temperature_2m_max,temperature_2m_min" +
    "&current=temperature_2m,weather_code&temperature_unit=" +
    unit;

  xhrRequest(url, "GET", callback);
}

function locationSuccess(pos) {
  var latitude = pos.coords.latitude;
  var longitude = pos.coords.longitude;
  getWeatherFromLocation(latitude, longitude, function (err, resp) {
    if (err) {
      notifyWatchOfError(ERRORS.http);
      return;
    }

    var data;

    try {
      data = JSON.parse(resp);
    } catch (e) {
      console.log("parse error:", e);
      notifyWatchOfError(ERRORS.json_parse);
      return;
    }

    var temperature_f = Math.round(data.current.temperature_2m);
    var weather_code = data.current.weather_code;
    var condition = weatherCodeToText(weather_code);
    var high_f = Math.round(data.daily.temperature_2m_max[0]);
    var low_f = Math.round(data.daily.temperature_2m_min[0]);

    // We only show solar times in the future so that is why
    // we get today and tomorrow's sunrise/sunset. If 'now' is past
    // sunrise, we show tomorrow's sunrise time. Same with sunset.
    var now = new Date();
    var suncalcTimesToday = suncalc.getTimes(now, latitude, longitude);

    var tomorrow = new Date();
    tomorrow.setDate(tomorrow.getDate() + 1);
    var suncalcTimesTomorrow = suncalc.getTimes(tomorrow, latitude, longitude);

    // Moon phase: 0=new, 14=full. Use 28 buckets to match Nerd Font icons.
    var illumination = suncalc.getMoonIllumination(now);
    var moon_phase = Math.round(illumination.phase * 27);

    var sunriseToday = new Date(suncalcTimesToday.sunrise);
    var sunsetToday = new Date(suncalcTimesToday.sunset);
    var sunriseTomorrow = new Date(suncalcTimesTomorrow.sunrise);
    var sunsetTomorrow = new Date(suncalcTimesTomorrow.sunset);

    var nextSunrise = now < sunriseToday ? sunriseToday : sunriseTomorrow;
    var nextSunset = now < sunsetToday ? sunsetToday : sunsetTomorrow;

    var weatherData = {
      temperature_f,
      condition,
      weather_code,
      high_f,
      low_f,
      sunrise: Math.floor(nextSunrise.getTime() / 1000),
      sunset: Math.floor(nextSunset.getTime() / 1000),
      moon_phase
    };

    console.log("weatherData:", JSON.stringify(weatherData));

    Pebble.sendAppMessage(
      weatherData,
      function (e) {
        console.log("sendAppMessage successful", e);
      },
      function (e) {
        console.log("sendAppMessage error:", e);
      }
    );
  });
}

function locationError(err) {
  console.log("Error requesting location!", err);
  notifyWatchOfError(ERRORS.location);
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, {
    enableHighAccuracy: true,
    timeout: 15000,
    maximumAge: 0 // Force GPS instead of cached wifi location.
  });
}

// Listen for when the watchface is opened
Pebble.addEventListener("ready", function (e) {
  console.log("PebbleKit JS ready!");
});

// Listen for when an AppMessage is received
Pebble.addEventListener("appmessage", function (e) {
  console.log("AppMessage received!");

  getWeather();
});
