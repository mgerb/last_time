module.exports = [
  {
    type: "heading",
    defaultValue: "Last Time"
  },
  {
    type: "text",
    defaultValue: "A multifunctional Pebble watchface."
  },
  {
    type: "text",
    defaultValue: "https://github.com/mgerb/last_time"
  },
  {
    type: "text",
    defaultValue:
      "If you encounter a bug, please open a new issue on the GitHub page."
  },
  {
    type: "section",
    items: [
      {
        type: "select",
        messageKey: "config_temperature",
        defaultValue: "f",
        label: "Temperature unit",
        options: [
          {
            label: "Fahrenheit (F)",
            value: "f"
          },
          {
            label: "Celsius (C)",
            value: "c"
          }
        ]
      },
      {
        type: "select",
        messageKey: "config_date_format",
        defaultValue: "MM-DD",
        label: "Date format",
        options: [
          {
            label: "MM-DD",
            value: "MM-DD"
          },
          {
            label: "YYYY-MM-DD",
            value: "YYYY-MM-DD"
          },
          {
            label: "MM-DD-YYYY",
            value: "MM-DD-YYYY"
          },
          {
            label: "DD-MM-YYYY",
            value: "DD-MM-YYYY"
          }
        ]
      },
      {
        type: "select",
        messageKey: "config_date_separator",
        defaultValue: "-",
        label: "Date separator",
        options: [
          {
            label: "-",
            value: "-"
          },
          {
            label: "/",
            value: "/"
          },
          {
            label: ".",
            value: "."
          }
        ]
      },
      {
        type: "toggle",
        messageKey: "config_vibrate_disconnect",
        label: "Vibrate on connect/disconnect",
        defaultValue: true
      },
      {
        type: "toggle",
        messageKey: "config_vibrate_top_hour",
        label: "Vibrate on the hour",
        defaultValue: false
      },
      {
        type: "slider",
        messageKey: "config_weather_update_interval",
        defaultValue: 30,
        label: "Weather update interval (minutes)",
        description: "",
        min: 1,
        max: 60,
        step: 1
      },
      {
        type: "toggle",
        messageKey: "config_show_steps",
        label: "Show steps (requires health)",
        defaultValue: true,
        description:
          "Disable this if you get popups due to health being disabled."
      },
      {
        type: "toggle",
        messageKey: "config_show_heart_rate",
        label: "Show heart rate (supported watches only)",
        defaultValue: false,
        description:
          "Show heart rate where the Bluetooth icon normally appears (Bluetooth icon moves to the top)."
      }
    ]
  },
  {
    type: "heading",
    defaultValue: "Secondary time",
    size: 2
  },
  {
    type: "section",
    items: [
      {
        type: "slider",
        messageKey: "config_utc_offset_minutes",
        defaultValue: 0,
        label: "UTC offset (minutes)",
        description: "e.g. UTC-5 = -300, UTC+5:30 = 330, etc.",
        min: -720,
        max: 840,
        step: 15
      },
      {
        type: "toggle",
        messageKey: "config_utc_time_24h",
        label: "24h clock",
        defaultValue: true
      }
    ]
  },
  {
    type: "heading",
    defaultValue: "Colors",
    size: 2
  },
  {
    type: "section",
    items: [
      {
        type: "color",
        messageKey: "config_text_color",
        defaultValue: "ffffff",
        label: "Text color"
      },
      {
        type: "color",
        messageKey: "config_text_color_secondary",
        defaultValue: "000000",
        label: "Secondary text color"
      },
      {
        type: "color",
        messageKey: "config_bg_color",
        defaultValue: "000000",
        label: "Background color"
      },
      {
        type: "color",
        messageKey: "config_bg_color_secondary",
        defaultValue: "ffffff",
        label: "Secondary background color"
      }
    ]
  },
  {
    type: "submit",
    defaultValue: "Save settings"
  }
];
