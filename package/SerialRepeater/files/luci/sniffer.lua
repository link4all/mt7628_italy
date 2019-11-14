module("luci.controller.admin.sniffer", package.seeall)

function index()
	entry({"admin", "sniffer"}, alias("admin", "sniffer", "configure"), _("集控器"), 70).index = true
	entry({"admin", "sniffer", "configure"}, cbi("admin_sniffer/configure"), _("设置"), 1)
	entry({"admin", "sniffer", "syslog"}, call("action_syslog"), _("日志"), 2)
end

function action_syslog()
	local syslog = luci.sys.snfrlog()
	luci.template.render("admin_sniffer/syslog", {syslog=syslog})
end
