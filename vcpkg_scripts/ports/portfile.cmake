vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO eladmaimoni/Eureka
    REF v0.0.4
    SHA512 237aba5023db47e52b3ebd57a32a84c02a91d7f333b8d35dd2e37c6ae71db4c09afdff57e9a41285e3a5dd61b965ad214e178700efc58a430b70cd011509b664
    HEAD_REF master
)



vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

#vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Ogg PACKAGE_NAME ogg)

#file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_copy_pdbs()
vcpkg_fixup_pkgconfig()

#vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")
