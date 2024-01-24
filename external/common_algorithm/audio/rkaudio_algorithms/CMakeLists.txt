#1.cmake version
cmake_minimum_required(VERSION 3.2)

#2.project name
project(RKAUDIOALGORITHMS)

# ----------------------------------------------------------------------------
#  install resource
# ----------------------------------------------------------------------------
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/conf/
	DESTINATION "vqefiles"
	FILES_MATCHING PATTERN "*.json"
)

# ----------------------------------------------------------------------------
#  install headers
# ----------------------------------------------------------------------------
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
	DESTINATION "include"
	FILES_MATCHING PATTERN "*.h"
)

# ----------------------------------------------------------------------------
#  install libs
# ----------------------------------------------------------------------------
if (USE_UCLIBC)
	if(RKAPPLUS_AED_ENABLE OR RKAPPLUS_ENABLE)
		install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/uclibc/librkaudio_common.so
				${CMAKE_CURRENT_SOURCE_DIR}/uclibc/librkaudio_common.a
			DESTINATION "lib"
		)
	endif()

	if(RKAPPLUS_ENABLE)
		if (RKAPPLUS_WAKEUP_ENABLE)
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/uclibc/libaec_bf_process_wakeup.so
				DESTINATION "lib"
				RENAME libaec_bf_process.so
			)
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/uclibc/libaec_bf_process_wakeup.a
				DESTINATION "lib"
				RENAME libaec_bf_process.a
			)
		else()
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/uclibc/libaec_bf_process.so
				${CMAKE_CURRENT_SOURCE_DIR}/uclibc/libaec_bf_process.a
				DESTINATION "lib"
			)
		endif()
	endif()

	if (RKAPPLUS_AED_ENABLE)
		install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/uclibc/librkaudio_detect.so
				${CMAKE_CURRENT_SOURCE_DIR}/uclibc/librkaudio_detect.a
			DESTINATION "lib"
		)
	endif()

endif()

if (USE_32BIT)
	if(RKAPPLUS_AED_ENABLE OR RKAPPLUS_ENABLE)
		install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib32/librkaudio_common.so
				${CMAKE_CURRENT_SOURCE_DIR}/lib32/librkaudio_common.a
			DESTINATION "lib"
		)
	endif()

	if(RKAPPLUS_ENABLE)
	    if (RKAPPLUS_WAKEUP_ENABLE)
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib32/libaec_bf_process_wakeup.so
				DESTINATION "lib"
				RENAME libaec_bf_process.so
			)
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib32/libaec_bf_process_wakeup.a
				DESTINATION "lib"
				RENAME libaec_bf_process.a
			)
		else()
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib32/libaec_bf_process.so
				${CMAKE_CURRENT_SOURCE_DIR}/lib32/libaec_bf_process.a
				DESTINATION "lib"
			)
		endif()
	endif()

	if (RKAPPLUS_AED_ENABLE)
		install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib32/librkaudio_detect.so
				${CMAKE_CURRENT_SOURCE_DIR}/lib32/librkaudio_detect.a
			DESTINATION "lib"
	)
	endif()

endif()

if (USE_64BIT)
	if(RKAPPLUS_AED_ENABLE OR RKAPPLUS_ENABLE)
		install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib64/librkaudio_common.so
				${CMAKE_CURRENT_SOURCE_DIR}/lib64/librkaudio_common.a
			DESTINATION "lib"
		)
	endif()

	if(RKAPPLUS_ENABLE)
		if (RKAPPLUS_WAKEUP_ENABLE)
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib32/libaec_bf_process_wakeup.so
				DESTINATION "lib"
				RENAME libaec_bf_process.so
			)
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib64/libaec_bf_process_wakeup.a
				DESTINATION "lib"
				RENAME libaec_bf_process.a
			)
		else()
			install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib64/libaec_bf_process.so
				${CMAKE_CURRENT_SOURCE_DIR}/lib64/libaec_bf_process.a
				DESTINATION "lib"
			)
		endif()
	endif()

	if (RKAPPLUS_AED_ENABLE)
		install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib64/librkaudio_detect.so
				${CMAKE_CURRENT_SOURCE_DIR}/lib64/librkaudio_detect.a
			DESTINATION "lib"
	)
	endif()
endif()
