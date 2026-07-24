module.exports = [
  {
    type: "heading",
    defaultValue: "Minitrix2 Configuration",
  },
  {
    type: "text",
    defaultValue: "Calibrate your Omnimatrix device",
  },

  {
    type: "section",
    items: [
      {
        type: "heading",
        defaultValue: "Dial",
      },
      {
        type: "radiogroup",
        defaultValue: "c",
        label: "Dial style",
        messageKey: "PrefDialMode",
        options: [
          {
            label: "Classic",
            value: "c",
          },
          {
            label: "Alien Force / Ultimate Alien",
            value: "u",
          },
          {
            label: "Omniverse",
            value: "o",
          },
        ],
      },
      {
        type: "toggle",
        label: "Hide text",
        description: "To see the time and information, shake your watch",
        defaultValue: false,
        messageKey: "PrefHideUI",
      },

      {
        capabilities: ["RECT"],
        type: "radiogroup",
        defaultValue: "t",
        label: "Classic: Digital clock direction",
        messageKey: "PrefRectClassicClockLtR",
        options: [
          {
            label: "Top-to-bottom",
            value: "f",
          },
          {
            label: "Left-to-right",
            value: "t",
          },
        ],
      },

      {
        capabilities: ["COLOR"],
        type: "section",
        items:
          [
            {
              capabilities: ["COLOR"],
              type: "heading",
              size: 4,
              defaultValue: "Colors",
            },
            {
              type: "toggle",
              label: "High contrast text",
              description:
                "If you set a custom color, text will always be high contrast",
              defaultValue: false,
              messageKey: "PrefHighContrast",
            },
            {
              capabilities: ["PLATFORM_EMERY"],
              type: "toggle",
              label: "Colorful backlight",
              description:
                "If enabled, the watch's backlight will match the color of the jewel or status indicator",
              defaultValue: false,
              messageKey: "PrefColorfulBacklight",
            },
            {
              type: "toggle",
              label: "Override main color",
              defaultValue: false,
              messageKey: "PrefDoColorOverride",
            },
            {
              type: "color",
              label: "Custom color",
              defaultValue: "0xFF0000",
              messageKey: "PrefOverrideColor",
            },
        ],
      },

      {
        type: "section",
        items: [
          {
            type: "heading",
            size: 4,
            defaultValue: "Features",
          },

          {
            capabilities: ["PLATFORM_EMERY"],
            type: "slider",
            messageKey: "PrefLowBattery30D",
            defaultValue: 6,
            description: "Defaults to 6%",
            label: "Low battery threshold",
            min: 0,
            max: 100,
            step: 1,
            group: "LowBattery"
          },
          {
            capabilities: ["PLATFORM_FLINT"],
            type: "slider",
            messageKey: "PrefLowBattery30D",
            defaultValue: 6,
            description: "Defaults to 6%",
            label: "Low battery threshold",
            min: 0,
            max: 100,
            step: 1,
            group: "LowBattery"
          },

          {
            capabilities: ["PLATFORM_GABBRO"],
            type: "slider",
            messageKey: "PrefLowBattery14D",
            defaultValue: 8,
            description: "Defaults to 8%",
            label: "Low battery threshold",
            min: 0,
            max: 100,
            step: 1,
            group: "LowBattery"
          },

          {
            capabilities: ["NOT_PLATFORM_FLINT", "NOT_PLATFORM_EMERY", "NOT_PLATFORM_GABBRO", "NOT_PLATFORM_CHALK"],
            type: "slider",
            messageKey: "PrefLowBattery7D",
            defaultValue: 20,
            description: "Defaults to 20%",
            label: "Low battery threshold",
            min: 0,
            max: 100,
            step: 10,
            group: "LowBattery"
          },

          {
            capabilities: ["PLATFORM_CHALK"],
            type: "slider",
            messageKey: "PrefLowBattery2D",
            defaultValue: 40,
            description: "Defaults to 40%",
            label: "Low battery threshold",
            min: 0,
            max: 100,
            step: 10,
            group: "LowBattery"
          },
        ],
      },
    ],
  },

  {
    type: "submit",
    defaultValue: "Save",
  },
];
