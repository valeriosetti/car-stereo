################################################################################
#
# hello
#
################################################################################

DABON_CLI_VERSION = 1.0
DABON_CLI_SITE = ./package/dabon-cli/src
DABON_CLI_SITE_METHOD = local

define DABON_CLI_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define DABON_CLI_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/dabon-cli $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
