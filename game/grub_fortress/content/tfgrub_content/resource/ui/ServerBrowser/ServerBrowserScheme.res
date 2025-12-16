Scheme
{
	Colors
	{
		// Server Browser Colors
		"ServerBrowser.Background"			"32 32 32 255"
		"ServerBrowser.Panel"				"40 40 40 255"
		"ServerBrowser.ServerPanel"			"45 45 45 255"
		"ServerBrowser.Text"				"200 200 200 255"
		"ServerBrowser.TextBright"			"240 240 240 255"
		"ServerBrowser.TextDim"				"160 160 160 255"
		"ServerBrowser.Border"				"60 60 60 255"
		"ServerBrowser.Selected"			"75 75 75 255"
		"ServerBrowser.ButtonHover"			"70 70 70 255"
		
		// Status Colors
		"ServerBrowser.WarningRed"			"255 120 120 255"
		"ServerBrowser.GoodGreen"			"120 255 120 255"
		"ServerBrowser.CheatsRed"			"255 80 80 255"
		"ServerBrowser.DisabledYellow"		"255 200 80 255"
		
		// Tab Colors
		"ServerBrowser.TabActive"			"55 55 55 255"
		"ServerBrowser.TabInactive"			"40 40 40 255"
		"ServerBrowser.TabText"				"200 200 200 255"
		"ServerBrowser.TabTextInactive"		"140 140 140 255"
	}
	
	BaseSettings
	{
		// Server Browser specific settings
		"ServerBrowser.BgColor"				"ServerBrowser.Background"
		"ServerBrowser.FgColor"				"ServerBrowser.Text"
		"ServerBrowser.ListPanel.BgColor"	"ServerBrowser.Panel"
		"ServerBrowser.ListPanel.OutlineFocusedColor" "ServerBrowser.Border"
		"ServerBrowser.ListPanel.OutlineColor" "ServerBrowser.Border"
		"ServerBrowser.ListPanel.SelectedTextColor" "ServerBrowser.TextBright"
		"ServerBrowser.ListPanel.SelectedBgColor" "ServerBrowser.Selected"
		"ServerBrowser.ListPanel.SelectedOutlineColor" "ServerBrowser.Border"
	}
	
	Fonts
	{
		"ServerBrowserTitle"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"24"
				"weight"	"900"
				"additive"	"0"
				"antialias" "1"
			}
		}
		
		"ServerBrowserBold"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"16"
				"weight"	"700"
				"additive"	"0"
				"antialias" "1"
			}
		}
		
		"ServerBrowserSmall"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"12"
				"weight"	"400"
				"additive"	"0"
				"antialias" "1"
			}
		}
	}
	
	Borders
	{
		"ServerBrowserBorder"
		{
			"inset" "0 0 0 0"
			Left
			{
				"1"
				{
					"color" "ServerBrowser.Border"
					"offset" "0 0"
				}
			}
			Right
			{
				"1"
				{
					"color" "ServerBrowser.Border"
					"offset" "0 0"
				}
			}
			Top
			{
				"1"
				{
					"color" "ServerBrowser.Border"
					"offset" "0 0"
				}
			}
			Bottom
			{
				"1"
				{
					"color" "ServerBrowser.Border"
					"offset" "0 0"
				}
			}
		}
	}
}
