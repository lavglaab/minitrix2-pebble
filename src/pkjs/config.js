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
    ],
  },

  {
    type: "submit",
    defaultValue: "Save",
  },
];
