include $(TOPDIR)/rules.mk

PKG_NAME:=SerialRepeater
PKG_VERSION:=1.00.00
PKG_RELEASE:=1

PKG_LICENSE:=BSD-3c
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/SerialRepeater
  SUBMENU:=Service
  SECTION:=IoT Add-On
  CATEGORY:=IoT Add-On
  TITLE:=SerialRepeater
  DEPENDS:=+glib2 +libevent2 +libcurl +libcares +libstdcpp +libuci +libubox +libmosquitto +libsqlite3 +libmodbus
  MAINTAINER:=Newton Jiang <newtonjiang.cn@gmail.com>
endef

define Package/SerialRepeater/description
endef

define Build/Configure
endef

define Package/SerialRepeater/config
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	PACKAGE_VERSION="$(PKG_VERSION)"

define Package/SerialRepeater/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/SerialRepeater.init $(1)/etc/init.d/SerialRepeater
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/SerialRepeater $(1)/etc/config/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/SerialRepeater $(1)/usr/bin/
	$(INSTALL_BIN) ./files/SRLauncher $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller/admin
	$(CP) ./files/luci/sniffer.lua $(1)/usr/lib/lua/luci/controller/admin/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(CP) -Rfp ./files/luci/admin_sniffer $(1)/usr/lib/lua/luci/model/cbi/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/admin_sniffer
	$(CP) -Rfp ./files/luci/syslog.htm $(1)/usr/lib/lua/luci/view/admin_sniffer/
	$(CP)  ./files/luci/sys.lua $(1)/usr/lib/lua/luci/

endef

$(eval $(call BuildPackage,SerialRepeater))
