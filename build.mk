TOPDIR:=${CURDIR}
PKG_BUILD_DIR:=.
PKG_INSTALL_DIR=${CURDIR}

include $(TOPDIR)/../include/top.mk

CFLAGS  = ${TARGET_CFLAGS}
LDFLAGS = ${TARGET_LDFLAGS}

CFLAGS  += -D_GNU_SOURCE
LDFLAGS += -Wl,--rpath -Wl, -L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib -L$(STAGING_DIR)/lib/mpp
LDFLAGS += -Wl,-Bdynamic -lspeex -lpthread -Wl,-Bstatic $(STAGING_DIR)/lib/libmpi.a $(STAGING_DIR)/lib/libVoiceEngine.a $(STAGING_DIR)/lib/libdnvqe.a $(STAGING_DIR)/lib/libupvqe.a  $(STAGING_DIR)/lib/libisp.a $(STAGING_DIR)/lib/lib_hiae.a $(STAGING_DIR)/lib/lib_hiaf.a $(STAGING_DIR)/lib/lib_hiawb.a -Wl,-Bdynamic

CROSS_TOOLCHAIN = ${TARGET_CROSS}

define Build/Compile
	#LDFLAGS += "-WL, --rpath -Wl -L$(STAGING_DIR)/usr/lib" 
	$(MAKE) -C $(PKG_BUILD_DIR) all install DESTDIR=$(PKG_INSTALL_DIR)
endef

define Build/InstallDev
	#$(INSTALL_DIR) $(1)/include
	$(CP) $(PKG_INSTALL_DIR)/include/IPinterCom.h $(1)/include/
	#$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_INSTALL_DIR)/*.so* $(1)/lib/
	#$(INSTALL_DIR) $(1)/lib/pkgconfig
	#$(CP) $(PKG_INSTALL_DIR)/usr/local/lib/pkgconfig/*.pc $(1)/lib/pkgconfig/
endef

define Package/IPinterCom/install
	$(CP) $(PKG_INSTALL_DIR)/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/tools/global_cfg $(1)/usr/bin
	#$(CP) $(PKG_INSTALL_DIR)/script/config $(1)/etc/
endef

all:
	$(call Build/Compile)
	$(call Build/InstallDev, $(STAGING_DIR))
	$(call Package/IPinterCom/install, $(TARGET_INSTALL_DIR))

install:
	$(call Build/Compile)
	$(call Build/InstallDev, $(STAGING_DIR))
	$(call Package/IPinterCom/install, $(TARGET_INSTALL_DIR))

.PHONY:all install
