// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/Layouts/HUD/WantedInfo.layout

class OVT_WantedInfoWidgets
{
	static const ResourceName s_sLayout = "{472DF3D2F7500FEB}UI/Layouts/HUD/WantedInfo.layout";
	ResourceName GetLayout() { return s_sLayout; }

	VerticalLayoutWidget m_WantedInfoPanel;

	HorizontalLayoutWidget m_WantedLevel;

	RichTextWidget m_WantedText;

	bool Init(Widget root)
	{
		m_WantedInfoPanel = VerticalLayoutWidget.Cast(root.FindAnyWidget("m_WantedInfoPanel"));

		m_WantedLevel = HorizontalLayoutWidget.Cast(root.FindAnyWidget("m_WantedLevel"));

		m_WantedText = RichTextWidget.Cast(root.FindAnyWidget("m_WantedText"));

		return true;
	}
}
