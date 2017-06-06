TOPDIR:=${CURDIR}
PKG_BUILD_DIR:=.
PKG_INSTALL_DIR=${CURDIR}

include $(TOPDIR)/../include/top.mk

CFLAGS  = ${TARGET_CFLAGS}
LDFLAGS = ${TARGET_LDFLAGS}

CFLAGS  += -D_GNU_SOURCE
LDFLAGS += -lspeex -lpthread

CROSS_TOOLCHAIN = ${TARGET_CROSS}

define Build/Compile
	#LDFLAGS += "-WL, --rpath -Wl -L$(STAGING_DIR)/usr/lib" \ 
	#$(MAKE) -C $(PKG_BUILD_DIR) all
	$(MAKE) -C $(PKG_BUILD_DIR) all install DESTDIR=$(PKG_INSTALL_DIR)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/include
	$(CP) $(PKG_INSTALL_DIR)/usr/local/include/* $(1)/include/
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/local/lib/*.so* $(1)/lib/
	$(INSTALL_DIR) $(1)/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/local/lib/pkgconfig/*.pc $(1)/lib/pkgconfig/
endef

define Package/libspeex/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/local/lib/*.so* $(1)/usr/lib/
endef

all:
	$(call Build/Compile)

install:
	#$(call Build/Compile)
	#$(call Build/InstallDev, $(STAGING_DIR))
	#$(call Package/libspeex/install, $(TARGET_INSTALL_DIR))

.PHONY:all install
