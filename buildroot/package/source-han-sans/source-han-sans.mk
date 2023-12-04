SOURCE_HAN_SANS_VERSION = 2.001R
SOURCE_HAN_SANS_SITE = https://raw.githubusercontent.com/adobe-fonts/source-han-sans/$(SOURCE_HAN_SANS_VERSION)/SubsetOTF

include $(sort $(wildcard package/source-han-sans/*/*.mk))
