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
                "messageKey": "DialMode",
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
                "messageKey": "HideUI"
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
                "messageKey": "HighContrast"
            },
            {
                "capabilities": ["COLOR"],
                "type": "toggle",
                "label": "Override main color",
                "defaultValue": false,
                "messageKey": "DoColorOverride"
            },
            {
                "capabilities": ["COLOR"],
                "type": "color",
                "label": "Custom color",
                "defaultValue": "0xFF0000",
                "messageKey": "OverrideColor"
            }
        ]
    },

    {
        "type": "submit",
        "defaultValue": "Save"
    }
]