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
      "If you encounter a bug, please open a new issue on the Github page."
  },
  {
    type: "section",
    items: [
      {
        type: "select",
        messageKey: "config_temperature",
        defaultValue: "f",
        label: "Temperature Unit",
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
        label: "Date Format",
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
        label: "Date Separator",
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
        label: "Weather Update Interval (minutes)",
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
      }
    ]
  },
  {
    type: "submit",
    defaultValue: "Save Settings"
  }
];
