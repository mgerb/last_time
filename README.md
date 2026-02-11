<h1 align="center">Last Time</h1>
<p align="center">A multifunctional Pebble watchface.</p>
<p align="center">
    <a href="https://apps.rebble.io/en_US/application/6953664f91ea9d0009ebd734">
        https://apps.rebble.io/en_US/application/6953664f91ea9d0009ebd734
    </a>
</p>

<p align="center">
    <img src="./readme/last_time.png" alt="Description">
</p>

## Features

- Time, date, day of week
- Weather (Open-Meteo)
  - Text description
  - Weather icons (day/night)
- Temperature (°F/°C)
- Steps
- Battery level
  - Percentage
  - Icon
- Bluetooth status (connected/disconnected)
- Vibrate on Bluetooth disconnect
- Vibrate on the hour
- UTC time
- Sunrise/sunset
  - Displays the upcoming sunrise/sunset times.
    For example, if the current time is a few minutes
    past sunset, then the sunset time displayed will
    be for the following day.
- Moon phase

## Settings

- Temperature Unit
  - °F
  - °C
- Date Format
- Date Separator
- Vibrate on connect/disconnect
- Vibrate on the hour
- Weather Update Interval (minutes)

## Watches

- Pebble 2 Duo
- Pebble Time 2 (not yet supported)

## Weather

| Description                       | Icon (day)                               | Icon (night)                               |
| --------------------------------- | ---------------------------------------- | ------------------------------------------ |
| Clear / Mainly clear              | ![](readme/weather/clear_day.png)        | ![](readme/weather/clear_night.png)        |
| Partly cloudy / Overcast          | ![](readme/weather/overcast_day.png)     | ![](readme/weather/overcast_night.png)     |
| Fog / Icy fog                     | ![](readme/weather/fog_day.png)          | ![](readme/weather/fog_night.png)          |
| Drizzle / Rain / Showers          | ![](readme/weather/rain_day.png)         | ![](readme/weather/rain_night.png)         |
| Freezing drizzle / Freezing rain  | ![](readme/weather/rain_snow_day.png)    | ![](readme/weather/rain_snow_night.png)    |
| Snow / Snow grains / Snow showers | ![](readme/weather/snow_day.png)         | ![](readme/weather/snow_night.png)         |
| Thunderstorm                      | ![](readme/weather/thunderstorm_day.png) | ![](readme/weather/thunderstorm_night.png) |
| Hail storm                        | ![](readme/weather/hail_day.png)         | ![](readme/weather/hail_night.png)         |

## Moon Phases

| Abbreviation | Phase               | Icon                      |
| ------------ | ------------------- | ------------------------- |
| NEW          | New Moon            | ![](readme/moon/NEW.png)  |
| WXC1         | Waxing Crescent (1) | ![](readme/moon/WXC1.png) |
| WXC2         | Waxing Crescent (2) | ![](readme/moon/WXC2.png) |
| WXC3         | Waxing Crescent (3) | ![](readme/moon/WXC3.png) |
| WXC4         | Waxing Crescent (4) | ![](readme/moon/WXC4.png) |
| WXC5         | Waxing Crescent (5) | ![](readme/moon/WXC5.png) |
| WXC6         | Waxing Crescent (6) | ![](readme/moon/WXC6.png) |
| 1ST          | First Quarter       | ![](readme/moon/1ST.png)  |
| WXG1         | Waxing Gibbous (1)  | ![](readme/moon/WXG1.png) |
| WXG2         | Waxing Gibbous (2)  | ![](readme/moon/WXG2.png) |
| WXG3         | Waxing Gibbous (3)  | ![](readme/moon/WXG3.png) |
| WXG4         | Waxing Gibbous (4)  | ![](readme/moon/WXG4.png) |
| WXG5         | Waxing Gibbous (5)  | ![](readme/moon/WXG5.png) |
| WXG6         | Waxing Gibbous (6)  | ![](readme/moon/WXG6.png) |
| FULL         | Full Moon           | ![](readme/moon/FULL.png) |
| WNG1         | Waning Gibbous (1)  | ![](readme/moon/WNG1.png) |
| WNG2         | Waning Gibbous (2)  | ![](readme/moon/WNG2.png) |
| WNG3         | Waning Gibbous (3)  | ![](readme/moon/WNG3.png) |
| WNG4         | Waning Gibbous (4)  | ![](readme/moon/WNG4.png) |
| WNG5         | Waning Gibbous (5)  | ![](readme/moon/WNG5.png) |
| WNG6         | Waning Gibbous (6)  | ![](readme/moon/WNG6.png) |
| 3RD          | Third Quarter       | ![](readme/moon/3RD.png)  |
| WNC1         | Waning Crescent (1) | ![](readme/moon/WNC1.png) |
| WNC2         | Waning Crescent (2) | ![](readme/moon/WNC2.png) |
| WNC3         | Waning Crescent (3) | ![](readme/moon/WNC3.png) |
| WNC4         | Waning Crescent (4) | ![](readme/moon/WNC4.png) |
| WNC5         | Waning Crescent (5) | ![](readme/moon/WNC5.png) |
| WNC6         | Waning Crescent (6) | ![](readme/moon/WNC6.png) |

## Publishing a new release

- Update the "version" in `package.json`
- Commit and tag the new version. Only update the major/minor versions. Seems
  like Pebble does not support patch versions.
  - e.g. `git commit -m "v1.2.0 && git tag v1.2.0`
- `pebble clean && ./build-release.sh`
- Draft a new release and upload `./build/last_time.pbw` via web UI
