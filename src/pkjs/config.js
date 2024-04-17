module.exports = [
    {
        "type": "heading",
        "defaultValue": "Minitrix2 Configuration"
    },
    {
        "type": "text",
        "defaultValue": "Calibrate your Omnimatrix device"
    },

    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "UI"
            },
            {
                "type": "radiogroup",
                "defaultValue": "c",
                "label": "Dial mode",
                "messageKey": "PrefDialMode",
                "options": [
                    {
                        "label": "Classic",
                        "value": "c"
                    },
                    {
                        "label": "Alien Force / Ultimate Alien",
                        "value": "u"
                    },
                    {
                        "label": "Omniverse",
                        "value": "o"
                    }
                ]
            },
            {
                "type": "toggle",
                "label": "Hide UI",
                "description": "To see the time and information, shake your watch",
                "defaultValue": false,
                "messageKey": "PrefHideUI"
            },

            {
                "capabilities": ["COLOR"],
                "type": "heading",
                "size": 4,
                "defaultValue": "Colors"
            },
            {
                "capabilities": ["COLOR"],
                "type": "toggle",
                "label": "High contrast text",
                "description": "If you set a custom color, text will always be high contrast",
                "defaultValue": false,
                "messageKey": "PrefHighContrast"
            },
            {
                "capabilities": ["COLOR"],
                "type": "toggle",
                "label": "Override main color",
                "defaultValue": false,
                "messageKey": "PrefDoColorOverride"
            },
            {
                "capabilities": ["COLOR"],
                "type": "color",
                "label": "Custom color",
                "defaultValue": "0xFF0000",
                "messageKey": "PrefOverrideColor"
            }
        ]
    },

    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Features"
            },
            {
                "type": "toggle",
                "label": "Show weather",
                "description": "You will need to provide an OpenWeatherMap API token for this to work",
                "defaultValue": false,
                "messageKey": "PrefDoWeather"
            },
            {
                "type": "input",
                "label": "OpenWeatherMap API Token",
                "defaultValue": "",
                "attributes": { "placeholder": "Paste here..." },
                "messageKey": "WeatherToken"
            },
            {
                "type": "radiogroup",
                "defaultValue": "f",
                "label": "Weather units",
                "messageKey": "PrefWeatherUnits",
                "options": [
                    {
                        "label": "Farenheit",
                        "value": "f"
                    },
                    {
                        "label": "Celsius",
                        "value": "c"
                    }
                ]
            }
        ]
    },

    {
        "type": "submit",
        "defaultValue": "Save"
    }
]