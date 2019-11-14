-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

-- serial1_port '/dev/ttyUSB0'
-- serial2_port '/dev/ttyS1'
-- serial3_port '/dev/ttyS2'
-- serial4_port '/dev/ttyS3'
-- serial1_enable '1'
-- serial2_enable '0'
-- serial3_enable '0'
-- serial4_enable '0'

m = Map("SerialRepeater", translate("网关参数配置"))

ports_setting = m:section(NamedSection, "ports", "Repeater", translate("端口设置"))
ports_setting.addremove = false
ports_setting.anonymous = true

serial1_port = ports_setting:option(Value, "serial1_port", translate("端口号1"))
serial1_enable = ports_setting:option(ListValue, "serial1_enable", translate("端口1开关"))
serial1_enable:value("0", "关闭")
serial1_enable:value("1", "开启")

serial2_port = ports_setting:option(Value, "serial2_port", translate("端口号2"))
serial2_enable = ports_setting:option(ListValue, "serial2_enable", translate("端口2开关"))
serial2_enable:value("0", "关闭")
serial2_enable:value("1", "开启")

serial3_port = ports_setting:option(Value, "serial3_port", translate("端口号3"))
serial3_enable = ports_setting:option(ListValue, "serial3_enable", translate("端口3开关"))
serial3_enable:value("0", "关闭")
serial3_enable:value("1", "开启")

serial4_port = ports_setting:option(Value, "serial4_port", translate("端口号4"))
serial4_enable = ports_setting:option(ListValue, "serial4_enable", translate("端口4开关"))
serial4_enable:value("0", "关闭")
serial4_enable:value("1", "开启")

-- api_base 'http://192.168.101.220:10080'
-- mqtt_broker '47.106.71.14'
-- mqtt_port '1883'
-- mqtt_username 'public'
-- mqtt_password 'public'
-- data_interval '30'
-- data_retry '5'
-- data_batch '0'
-- sub_vendor '0'
-- project_prefix '0'
-- ctrl_room_no '101'

global_setting = m:section(NamedSection, "global", "Repeater", translate("运行参数"))
global_setting.addremove = false
global_setting.anonymous = true

api_base = global_setting:option(Value, "api_base", translate("API根地址"))
mqtt_broker = global_setting:option(Value, "mqtt_broker", translate("MQTT服务器地址"))
mqtt_port = global_setting:option(Value, "mqtt_port", translate("MQTT服务器端口"))
mqtt_username = global_setting:option(Value, "mqtt_username", translate("MQTT用户名"))
mqtt_password = global_setting:option(Value, "mqtt_password", translate("MQTT密码"))
data_interval = global_setting:option(Value, "data_interval", translate("数据上报间隔"))
data_retry = global_setting:option(Value, "data_retry", translate("上报失败重试"))
data_batch = global_setting:option(Value, "data_batch", translate("单次上报节点数量"))
sub_vendor = global_setting:option(Value, "sub_vendor", translate("代理编号"))
project_prefix = global_setting:option(Value, "project_prefix", translate("项目编号"))
ctrl_room_no = global_setting:option(Value, "ctrl_room_no", translate("机房编号"))

ipdev_setting = m:section(NamedSection, "ipdev", "Repeater", translate("IP设备参数"))
ipdev_setting.addremove = false
ipdev_setting.anonymous = true

ipdev_addr = ipdev_setting:option(Value, "ipdev_addr", translate("服务器地址"))
ipdev_port = ipdev_setting:option(Value, "ipdev_port", translate("端口"))

return m
