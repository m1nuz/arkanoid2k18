add_custom_target(demo_assets
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/extract_assets.py
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/extract_assets.py
    COMMENT "Check assets"
    )
