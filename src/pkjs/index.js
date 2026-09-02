const suncalc = require("./suncalc");

// Import the Clay package
var Clay = require("@rebble/clay");
// Load our Clay configuration file
var clayConfig = require("./config");
// Disable autoHandleEvents so that we can remove keys before sending
// data to the watch. We don't want to share all data with the watch.
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

const CLAY_SETTINGS_KEY = "clay-settings";

// These settings will not be sent to the watch. We only need them on the JS (phone) side.
const EXCLUDED_SETTING_KEYS = [
  "config_location_high_accuracy",
  "config_location_cache",
  "config_location_override",
  "config_location_latitude",
  "config_location_longitude"
];

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
    { type_weather: 1, error: code || ERRORS.unknown },
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

function getStoredSettings() {
  try {
    var settings = JSON.parse(localStorage.getItem(CLAY_SETTINGS_KEY)) || {};
    if (typeof settings !== "object" || Array.isArray(settings)) {
      return {};
    }
    return settings;
  } catch (e) {
    console.log("Unable to read stored settings:", e);
    return {};
  }
}

function parseCoordinate(value, min, max) {
  if (
    value === null ||
    typeof value === "undefined" ||
    (typeof value === "string" && value.trim() === "")
  ) {
    return null;
  }

  var coordinate = Number(value);
  if (!isFinite(coordinate) || coordinate < min || coordinate > max) {
    return null;
  }
  return coordinate;
}

function getLocationSettings() {
  var settings = getStoredSettings();
  return {
    highAccuracy: settings.config_location_high_accuracy !== false,
    cache: settings.config_location_cache === true,
    override: settings.config_location_override === true,
    latitude: parseCoordinate(settings.config_location_latitude, -90, 90),
    longitude: parseCoordinate(settings.config_location_longitude, -180, 180)
  };
}

function getWeatherForCoordinates(latitude, longitude) {
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
      type_weather: 1,
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
  var locationSettings = getLocationSettings();

  if (locationSettings.override) {
    if (
      locationSettings.latitude !== null &&
      locationSettings.longitude !== null
    ) {
      getWeatherForCoordinates(
        locationSettings.latitude,
        locationSettings.longitude
      );
      return;
    }

    console.log(
      "Location override is enabled, but the coordinates are invalid. Falling back to phone location."
    );
  }

  navigator.geolocation.getCurrentPosition(
    (pos) => {
      getWeatherForCoordinates(pos.coords.latitude, pos.coords.longitude);
    },
    locationError,
    {
      enableHighAccuracy: locationSettings.highAccuracy,
      timeout: 15000,
      maximumAge: locationSettings.cache ? Infinity : 0
    }
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener("ready", function (e) {
  console.log("PebbleKit JS ready!");
});

Pebble.addEventListener("showConfiguration", function () {
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener("webviewclosed", function (e) {
  if (!e || !e.response) {
    return;
  }

  var settings;
  try {
    settings = clay.getSettings(e.response, false);
  } catch (err) {
    console.log("Unable to save config data:", err);
    return;
  }

  EXCLUDED_SETTING_KEYS.forEach(function (key) {
    delete settings[key];
  });

  var watchSettings = Clay.prepareSettingsForAppMessage(settings);
  Pebble.sendAppMessage(
    watchSettings,
    function () {
      console.log("Sent config data to Pebble");
    },
    function (err) {
      console.log("Failed to send config data!", err);
    }
  );
});

// Listen for when an AppMessage is received
Pebble.addEventListener("appmessage", function (e) {
  console.log("AppMessage received!");

  getWeather();
});
