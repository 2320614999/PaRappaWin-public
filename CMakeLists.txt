cmake_minimum_required(VERSION 3.16)
project(PaRappaWin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

if(MSVC)
    add_compile_options(/utf-8)
endif()

# Windows SDK
set(CMAKE_SYSTEM_VERSION 10.0)

# Keep build outputs inside the public checkout.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Source files
set(SOURCES
    src/main.cpp
    src/app_config.cpp
    src/d3d11_renderer.cpp
    src/tim_decoder.cpp
    src/boot_logo.cpp
    src/int_loader.cpp
    src/resource_manager.cpp
    src/menu_scene.cpp
    src/pr/pr_main.cpp
    src/pr/pr_event.cpp
    src/pr/pr_sqevs1.cpp
    src/pr/pr_card.cpp
    src/pr/pr_memcard_backend.cpp
    src/pr/pr_overlay_loader.cpp
    src/pr/pr_scenes.cpp
    src/pr/pr_stage1_scorer_host.cpp
    src/pr/pr_stage1_scorer_host_direct.cpp
    src/pr/pr_stage1_scorer_direct.cpp
    src/pr/pr_stage1_rating_presentation_direct.cpp
    src/pr/pr_stage1_hud_presentation_direct.cpp
    src/pr/pr_stage1_lifecycle_direct.cpp
    src/pr/pr_stage1_lifecycle_executor_direct.cpp
    src/pr/pr_stage1_bootstrap_cd_request_direct.cpp
    src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp
    src/pr/pr_stage1_loader_direct.cpp
    src/pr/pr_stage1_loader_producer_adapter.cpp
    src/pr/pr_stage1_loader_memory_direct.cpp
    src/pr/pr_stage1_loader_cd_hal.cpp
    src/pr/pr_stage1_loader_gpu_hal.cpp
    src/pr/pr_stage1_loader_spu_hal.cpp
    src/pr/pr_stage1_lower_cd_producer_direct.cpp
    src/pr/pr_stage1_fail_prompt_direct.cpp
    src/pr/pr_stage1_overlay_script_text_direct.cpp
    src/pr/pr_stage1_rail_cursor_event_direct.cpp
    src/pr/pr_stage1_script_event_runtime_direct.cpp
    src/pr/pr_stage_event_direct.cpp
    src/pr/pr_stage1_runtime_slots_direct.cpp
    src/pr/pr_stage1_compact_rail_80024744_direct.cpp
    src/pr/pr_stage1_camera_motion_direct.cpp
    src/pr/pr_stage1_tod_cursor_direct.cpp
    src/pr/pr_stage_scene_submit_backend.cpp
    src/pr/pr_stage_scene_submit_direct.cpp
    src/pr/pr_stage_payload_bank_direct.cpp
    src/pr/pr_stage_status_bank_direct.cpp
    src/pr/pr_stage_status_bank_host_bridge_direct.cpp
    src/pr/pr_psx_event_frame_direct.cpp
    src/pr/pr_psx_vsync_direct.cpp
    src/pr/pr_psx_fast_sprite_submit_direct.cpp
    src/pr/pr_psx_gte_direct.cpp
    src/pr/pr_psx_gs_sprite_submit_direct.cpp
    src/pr/pr_psx_tmd_submit_direct.cpp
    src/pr/pr_psx_graph_owner_direct.cpp
    src/pr/pr_psx_clear_image_direct.cpp
    src/pr/pr_psx_dma_submit_direct.cpp
    src/pr/pr_psx_pad_direct.cpp
    src/pr/pr_stage1_save_card_hal_direct.cpp
    src/pr/pr_stage1_save_ui_directory_carrier_direct.cpp
    src/pr/pr_stage1_save_ui_host_bridge_direct.cpp
    src/pr/pr_stage1_save_ui_direct.cpp
    src/pr/pr_stage1_xa_cd_direct.cpp
    src/pr/pr_stage1_movie_text_direct.cpp
    src/pr/pr_stage1_movie_text_outer_loop_direct.cpp
    src/pr/pr_ss0_direct.cpp
    src/pr/pr_ss0_scene0_runtime_direct.cpp
    src/pr/pr_ss0_card_image_storage_direct.cpp
    src/pr/pr_ss0_word800916f0_direct.cpp
    src/pr/pr_ss0_state16_runtime_one_shot_direct.cpp
    src/pr/pr_ss0_state16_runtime_envelope_import_direct.cpp
    src/pr/pr_ss0_transition_direct.cpp
    src/pr/pr_ss0_directory_dispatcher_direct.cpp
    src/pr/pr_ss0_resource_audio_direct.cpp
    src/pr/pr_ss0_title_render_direct.cpp
    src/pr/pr_ss0_title_tmd_backend.cpp
    src/pr/pr_ss0_title_draw_backend.cpp
    src/pr/pr_ss0_title_draw_desc_direct.cpp
    src/pr/pr_ss0_title_packet_render_direct.cpp
    src/pr/pr_ss0_title_packet_commit_direct.cpp
    src/pr/pr_ss0_title_packet_plan_direct.cpp
    src/pr/pr_ss0_title_primitive_group_direct.cpp
    src/pr/pr_ss0_title_packet_work_direct.cpp
    src/pr/pr_ss0_title_transform_direct.cpp
    src/pr/pr_ss0_str_lifecycle_direct.cpp
    src/pr/pr_ss0_mdec_vlc_direct.cpp
    src/pr/pr_ss0_mdec_output_direct.cpp
    src/pr/pr_ss0_title_entry_prefix_direct.cpp
    src/pr/pr_ss0_title_initial_clock_direct.cpp
    src/pr/pr_ss0_title_hud_events_direct.cpp
    src/pr/pr_ss0_scene0_global_bindings_direct.cpp
    src/pr/pr_ss0_scene0_resource_ingress_direct.cpp
    src/pr/pr_ss0_scene0_int_load_direct.cpp
    src/pr/pr_ss0_scene0_int_side_effect_direct.cpp
    src/pr/pr_ss0_scene0_int_spu_direct.cpp
    src/pr/pr_ss0_scene0_int_gpu_direct.cpp
    src/pr/pr_ss0_scene0_int_renderer_direct.cpp
    src/pr/pr_ss0_scene0_shared_event_predispatch_direct.cpp
    src/pr/pr_ss0_card_memcard_handoff_direct.cpp
    src/pr/pr_ss0_stage_progress_bank_direct.cpp
    src/pr/pr_ss0_event_frame_loop_direct.cpp
    src/pr/pr_ss0_event_text_direct.cpp
    src/pr/pr_ss0_practice_lifecycle_direct.cpp
    src/pr/pr_ss0_directory_pages_render_direct.cpp
    src/pr/pr_ss0_event_backdrop_render_direct.cpp
    src/pr/pr_ss0_card_io_banner_render_direct.cpp
    src/pr/pr_ss0_hiscore_render_direct.cpp
    src/pr/pr_ss0_card_info_render_direct.cpp
    src/pr/pr_ss0_prompt_card_render_direct.cpp
    src/pr/pr_ss0_event4_prompt_render_direct.cpp
    src/pr/pr_scene_drawbuffer_direct.cpp
    src/pr/pr_scene_boot_worklist_direct.cpp
    src/pr/pr_scene_bootstrap_direct.cpp
    src/pr/pr_scene_entry_direct.cpp
    src/pr/pr_scene_entry_card_feedback_direct.cpp
    src/pr/pr_scene_entry_feedback_adapter_direct.cpp
    src/pr/pr_scene_entry_executor_direct.cpp
    src/pr/pr_scene1_entry_original_disc_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
    src/pr/pr_stage1_movie_segment_direct.cpp
    src/pr/pr_stage1_scene1_movie1_direct.cpp
    src/pr/pr_stage1_scene1_draw_backend.cpp
    src/pr/pr_stage1_vtext_direct.cpp
    src/pr/pr_stage1_live_hud.cpp
    src/pr/pr_stage1_p2_scorer_hud.cpp
    src/pr/pr_stage1_hd_subtitles.cpp
    src/pr/pr_stage1_texture_replacements.cpp
    src/pr/pr_stage_runner.cpp
    src/pr/pr_stage_runner_direct.cpp
    src/pr/pr_pad.cpp
    src/pr/pr_timer.cpp
    src/pr/pr_cd.cpp
    src/pr/pr_mem.cpp
    src/pr/pr_beat_chart.cpp
    src/pr/pr_psx_sprite_template_render.cpp
    src/pr/pr_ui_overlay.cpp
    src/pr/pr_tmd.cpp
    src/pr/pr_tmd_renderer.cpp
    src/pr/scene_event_parser.cpp
    src/pr/pr_stage1_overlay_parser.cpp
    src/pr/face_event_processor.cpp
    src/pr/pr_vram_atlas.cpp
    src/pr/pr_mime.cpp
    src/pr/pr_transition.cpp
    src/pr/pr_sfx.cpp
    src/pr/pr_vtext.cpp
    src/debug_server.cpp
    src/str_player.cpp
    src/str_parser.cpp
    src/mdec_decoder.cpp
    src/xa_decoder.cpp
    src/xa1_player.cpp
    src/audio_output.cpp
    src/wasapi_sink.cpp
    src/audio_engine.cpp
    src/vab_player.cpp
)

set(HEADERS
    src/d3d11_renderer.h
    src/tim_decoder.h
    src/boot_logo.h
    src/tim_data.h
    src/int_loader.h
    src/resource_manager.h
    src/menu_scene.h
    src/logger.h
    src/debug_server.h
    src/pr/pr_mime.h
    src/pr/pr_stage1_camera_motion_direct.h
    src/pr/pr_stage1_hud_text_bridge_direct.h
    src/pr/pr_stage1_lifecycle_direct.h
    src/pr/pr_stage1_lifecycle_executor_direct.h
    src/pr/pr_stage1_bootstrap_cd_request_direct.h
    src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.h
    src/pr/pr_stage1_loader_bootstrap_plan_direct.h
    src/pr/pr_stage1_loader_direct.h
    src/pr/pr_stage1_loader_producer_adapter.h
    src/pr/pr_stage1_loader_memory_direct.h
    src/pr/pr_stage1_loader_cd_hal.h
    src/pr/pr_stage1_loader_gpu_hal.h
    src/pr/pr_stage1_loader_spu_hal.h
    src/pr/pr_stage1_movie_text_direct.h
    src/pr/pr_stage1_movie_text_outer_loop_direct.h
    src/pr/pr_scene_drawbuffer_direct.h
    src/pr/pr_scene_boot_worklist_direct.h
    src/pr/pr_scene_bootstrap_direct.h
    src/pr/pr_scene_entry_direct.h
    src/pr/pr_scene_entry_executor_direct.h
    src/pr/pr_movie_segment_direct.h
    src/pr/pr_stage1_movie_segment_direct.h
    src/pr/pr_stage1_compact_rail_80024744_direct.h
    src/pr/pr_stage1_scene1_frame_driver_direct.h
    src/pr/pr_stage1_scene1_movie1_direct.h
    src/pr/pr_stage1_scene1_draw_backend.h
    src/pr/pr_stage1_scene1_render_router_direct.h
    src/pr/pr_stage1_texture_replacements.h
    src/pr/pr_stage1_save_card_hal_direct.h
    src/pr/pr_stage1_save_ui_host_bridge_direct.h
    src/pr/pr_stage1_save_ui_direct.h
    src/pr/pr_stage1_live_hud.h
    src/pr/pr_stage1_p2_scorer_hud.h
    src/pr/pr_stage1_hd_subtitles.h
    src/pr/pr_ss0_direct.h
    src/pr/pr_ss0_scene0_runtime_direct.h
    src/pr/pr_ss0_card_image_storage_direct.h
    src/pr/pr_ss0_word800916f0_direct.h
    src/pr/pr_ss0_state16_runtime_one_shot_direct.h
    src/pr/pr_ss0_state16_runtime_envelope_import_direct.h
    src/pr/pr_ss0_transition_direct.h
    src/pr/pr_ss0_directory_dispatcher_direct.h
    src/pr/pr_ss0_resource_audio_direct.h
    src/pr/pr_ss0_title_render_direct.h
    src/pr/pr_ss0_title_tmd_backend.h
    src/pr/pr_ss0_title_draw_backend.h
    src/pr/pr_ss0_title_draw_desc_direct.h
    src/pr/pr_ss0_title_packet_render_direct.h
    src/pr/pr_ss0_title_packet_commit_direct.h
    src/pr/pr_ss0_title_packet_plan_direct.h
    src/pr/pr_ss0_title_primitive_group_direct.h
    src/pr/pr_ss0_title_packet_work_direct.h
    src/pr/pr_ss0_title_transform_direct.h
    src/pr/pr_ss0_str_lifecycle_direct.h
    src/pr/pr_ss0_mdec_vlc_direct.h
    src/pr/pr_ss0_mdec_output_direct.h
    src/pr/pr_ss0_title_entry_prefix_direct.h
    src/pr/pr_ss0_title_initial_clock_direct.h
    src/pr/pr_ss0_title_hud_events_direct.h
    src/pr/pr_ss0_scene0_global_bindings_direct.h
    src/pr/pr_ss0_scene0_resource_ingress_direct.h
    src/pr/pr_ss0_scene0_int_load_direct.h
    src/pr/pr_ss0_scene0_int_side_effect_direct.h
    src/pr/pr_ss0_scene0_int_spu_direct.h
    src/pr/pr_ss0_scene0_int_gpu_direct.h
    src/pr/pr_ss0_scene0_int_renderer_direct.h
    src/pr/pr_ss0_scene0_shared_event_predispatch_direct.h
    src/pr/pr_ss0_card_memcard_handoff_direct.h
    src/pr/pr_ss0_stage_progress_bank_direct.h
    src/pr/pr_ss0_event_frame_loop_direct.h
    src/pr/pr_ss0_event_text_direct.h
    src/pr/pr_ss0_practice_lifecycle_direct.h
    src/pr/pr_ss0_directory_pages_render_direct.h
    src/pr/pr_ss0_event_backdrop_render_direct.h
    src/pr/pr_ss0_card_io_banner_render_direct.h
    src/pr/pr_ss0_hiscore_render_direct.h
    src/pr/pr_ss0_card_info_render_direct.h
    src/pr/pr_psx_text_glyph_metrics_direct.h
    src/pr/pr_ss0_prompt_card_render_direct.h
    src/pr/pr_ss0_event4_prompt_render_direct.h
    src/pr/pr_stage1_rating_presentation_direct.h
    src/pr/pr_stage1_scorer_host_direct.h
    src/pr/pr_stage_event_direct.h
    src/pr/pr_stage_scene_submit_backend.h
    src/pr/pr_stage_scene_submit_debug.h
    src/pr/pr_stage_scene_submit_constants.h
    src/pr/pr_stage_payload_bank_direct.h
    src/pr/pr_stage_status_bank_direct.h
    src/pr/pr_stage_status_bank_host_bridge_direct.h
    src/pr/pr_stage_runner_direct.h
    src/pr/pr_scene_entry_card_feedback_direct.h
    src/pr/pr_scene_entry_feedback_adapter_direct.h
    src/pr/pr_psx_event_frame_direct.h
    src/pr/pr_psx_vsync_direct.h
    src/pr/pr_psx_fast_sprite_submit_direct.h
    src/pr/pr_psx_gte_direct.h
    src/pr/pr_psx_gs_sprite_submit_direct.h
    src/pr/pr_psx_tmd_submit_direct.h
    src/pr/pr_psx_graph_owner_direct.h
    src/pr/pr_psx_pad_direct.h
    src/pr/pr_psx_sprite_template_render.h
    src/pr/pr_vram_atlas.h
)

# Executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})
if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE /STACK:8388608)
endif()

# Output stays in this checkout. Close its executable before rebuilding;
# never terminate another checkout's game or toolchain processes.

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    dxgi
    d3dcompiler
    shell32
    windowscodecs
    ws2_32
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(test_int
    src/test_int.cpp
    src/int_loader.cpp
)
target_include_directories(test_int PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(test_tim
    src/test_tim.cpp
    src/tim_decoder.cpp
)
target_include_directories(test_tim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(test_stage1_lower_cd_facts
    src/test_stage1_lower_cd_facts.cpp
    src/pr/pr_movie_segment_direct.cpp
    src/pr/pr_stage1_loader_cd_hal.cpp
    src/pr/pr_stage1_lower_cd_producer_direct.cpp
    src/pr/pr_stage1_xa_cd_direct.cpp
)
target_include_directories(test_stage1_lower_cd_facts PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(test_stage1_bootstrap_cd_request
    src/test_stage1_bootstrap_cd_request.cpp
    src/pr/pr_stage1_bootstrap_cd_request_direct.cpp
)
target_include_directories(test_stage1_bootstrap_cd_request PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_stage1_bootstrap_cd_request PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_stage1_bootstrap_cd_request"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_stage1_bootstrap_cd_request"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_stage1_bootstrap_cd_request"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_stage1_bootstrap_cd_request"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_stage1_bootstrap_cd_request"
)

add_executable(test_ss0_memcard_backend
    src/test_ss0_memcard_backend.cpp
    src/pr/pr_memcard_backend.cpp
)

add_executable(test_ss0_psx_tmd_submit
    src/test_ss0_psx_tmd_submit.cpp
    src/pr/pr_psx_tmd_submit_direct.cpp
)
target_include_directories(test_ss0_psx_tmd_submit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_psx_tmd_submit PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_tmd_submit"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_tmd_submit"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_tmd_submit"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_tmd_submit"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_tmd_submit"
)

add_executable(test_ss0_title_transform_direct
    src/test_ss0_title_transform_direct.cpp
    src/pr/pr_ss0_title_transform_direct.cpp
    src/pr/pr_psx_gte_direct.cpp
)
target_include_directories(test_ss0_title_transform_direct PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_title_transform_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_transform_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_transform_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_transform_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_transform_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_transform_direct"
)

add_library(test_ss0_title_rtpt3_runtime_compile OBJECT EXCLUDE_FROM_ALL
    src/pr/pr_main.cpp
    src/pr/pr_overlay_loader.cpp
    src/pr/pr_ss0_scene0_runtime_direct.cpp
    src/pr/pr_ss0_title_tmd_backend.cpp
    src/pr/pr_ss0_title_draw_backend.cpp
    src/pr/pr_ss0_title_draw_desc_direct.cpp
    src/pr/pr_ss0_title_packet_render_direct.cpp
    src/pr/pr_ss0_title_packet_commit_direct.cpp
    src/pr/pr_ss0_title_packet_plan_direct.cpp
    src/pr/pr_ss0_title_primitive_group_direct.cpp
    src/pr/pr_ss0_title_packet_work_direct.cpp
    src/pr/pr_ss0_title_transform_direct.cpp
    src/pr/pr_psx_gte_direct.cpp
)

add_executable(test_ss0_title_packet_work_direct
    src/test_ss0_title_packet_work_direct.cpp
    src/pr/pr_ss0_title_packet_work_direct.cpp
    src/pr/pr_ss0_transition_direct.cpp
    src/pr/pr_scene_drawbuffer_direct.cpp
    src/pr/pr_stage1_loader_memory_direct.cpp
    src/pr/pr_psx_graph_owner_direct.cpp
    src/pr/pr_psx_dma_submit_direct.cpp
    src/pr/pr_psx_tmd_submit_direct.cpp
    src/pr/pr_psx_fast_sprite_submit_direct.cpp
    src/pr/pr_psx_gs_sprite_submit_direct.cpp
    src/pr/pr_psx_gte_direct.cpp
)
target_include_directories(test_ss0_title_packet_work_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_title_packet_work_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_work_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_work_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_work_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_work_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_work_direct"
)

add_executable(test_ss0_title_draw_desc_direct
    src/test_ss0_title_draw_desc_direct.cpp
    src/pr/pr_ss0_title_draw_desc_direct.cpp
)
target_include_directories(test_ss0_title_draw_desc_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_title_draw_desc_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_draw_desc_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_draw_desc_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_draw_desc_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_draw_desc_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_draw_desc_direct"
)

add_executable(test_ss0_title_packet_plan_direct
    src/test_ss0_title_packet_plan_direct.cpp
    src/pr/pr_ss0_title_packet_plan_direct.cpp
    src/pr/pr_ss0_title_draw_desc_direct.cpp
    src/pr/pr_psx_tmd_submit_direct.cpp
)
target_include_directories(test_ss0_title_packet_plan_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_title_packet_plan_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_plan_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_plan_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_plan_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_plan_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_plan_direct"
)

add_executable(test_ss0_title_packet_commit_direct
    src/test_ss0_title_packet_commit_direct.cpp
    src/pr/pr_ss0_title_packet_commit_direct.cpp
)
target_include_directories(test_ss0_title_packet_commit_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_title_packet_commit_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_commit_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_commit_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_commit_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_commit_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_commit_direct"
)

add_executable(test_ss0_title_primitive_group_direct
    src/test_ss0_title_primitive_group_direct.cpp
    src/pr/pr_ss0_title_primitive_group_direct.cpp
    src/pr/pr_ss0_title_packet_commit_direct.cpp
    src/pr/pr_ss0_title_packet_plan_direct.cpp
    src/pr/pr_ss0_title_draw_desc_direct.cpp
    src/pr/pr_ss0_title_transform_direct.cpp
    src/pr/pr_psx_tmd_submit_direct.cpp
    src/pr/pr_psx_gte_direct.cpp
)

add_executable(test_ss0_title_packet_render_direct
    src/test_ss0_title_packet_render_direct.cpp
    src/pr/pr_ss0_title_packet_render_direct.cpp
)
target_include_directories(test_ss0_title_packet_render_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_title_packet_render_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_packet_render_direct"
)
target_include_directories(test_ss0_title_primitive_group_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_title_primitive_group_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_primitive_group_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_primitive_group_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_primitive_group_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_primitive_group_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_primitive_group_direct"
)
target_include_directories(test_ss0_title_rtpt3_runtime_compile PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(test_ss0_title_event_clock
    src/test_ss0_title_event_clock.cpp
    src/pr/pr_ss0_title_hud_events_direct.cpp
)
target_include_directories(test_ss0_title_event_clock PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_title_event_clock PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_event_clock"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_event_clock"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_event_clock"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_event_clock"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_event_clock"
)

add_executable(test_ss0_scene0_global_bindings_direct
    src/test_ss0_scene0_global_bindings_direct.cpp
    src/pr/pr_ss0_scene0_global_bindings_direct.cpp
)
target_include_directories(test_ss0_scene0_global_bindings_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_scene0_global_bindings_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_global_bindings_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_global_bindings_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_global_bindings_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_global_bindings_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_global_bindings_direct"
)
add_test(
    NAME test_ss0_scene0_global_bindings_direct
    COMMAND $<TARGET_FILE:test_ss0_scene0_global_bindings_direct>
)

add_executable(test_ss0_scene0_resource_ingress_direct
    src/test_ss0_scene0_resource_ingress_direct.cpp
    src/pr/pr_ss0_scene0_resource_ingress_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
    src/pr/pr_scene_entry_direct.cpp
    src/pr/pr_stage1_loader_cd_hal.cpp
)
target_include_directories(test_ss0_scene0_resource_ingress_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_scene0_resource_ingress_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_resource_ingress_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_resource_ingress_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_resource_ingress_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_resource_ingress_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_resource_ingress_direct"
)
add_test(
    NAME test_ss0_scene0_resource_ingress_direct
    COMMAND $<TARGET_FILE:test_ss0_scene0_resource_ingress_direct>
)

add_executable(test_ss0_scene1_entry_original_disc_direct
    src/test_ss0_scene1_entry_original_disc_direct.cpp
    src/pr/pr_scene1_entry_original_disc_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
    src/pr/pr_scene_entry_direct.cpp
    src/pr/pr_stage1_loader_cd_hal.cpp
)
target_include_directories(test_ss0_scene1_entry_original_disc_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_scene1_entry_original_disc_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene1_entry_original_disc_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene1_entry_original_disc_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene1_entry_original_disc_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene1_entry_original_disc_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene1_entry_original_disc_direct"
)

add_executable(test_ss0_scene0_int_load_direct
    src/test_ss0_scene0_int_load_direct.cpp
    src/pr/pr_ss0_scene0_int_load_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
    src/pr/pr_stage1_loader_cd_hal.cpp
)
target_include_directories(test_ss0_scene0_int_load_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_scene0_int_load_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_load_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_load_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_load_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_load_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_load_direct"
)
add_test(
    NAME test_ss0_scene0_int_load_direct
    COMMAND $<TARGET_FILE:test_ss0_scene0_int_load_direct>
)

add_executable(test_ss0_scene0_int_side_effect_direct
    src/test_ss0_scene0_int_side_effect_direct.cpp
    src/pr/pr_ss0_scene0_int_side_effect_direct.cpp
    src/pr/pr_ss0_scene0_int_load_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
    src/pr/pr_stage1_loader_cd_hal.cpp
    src/pr/pr_stage1_loader_memory_direct.cpp
)
target_include_directories(test_ss0_scene0_int_side_effect_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_scene0_int_side_effect_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_side_effect_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_side_effect_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_side_effect_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_side_effect_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_side_effect_direct"
)
add_test(
    NAME test_ss0_scene0_int_side_effect_direct
    COMMAND $<TARGET_FILE:test_ss0_scene0_int_side_effect_direct>
)

add_executable(test_ss0_scene0_int_spu_direct
    src/test_ss0_scene0_int_spu_direct.cpp
    src/pr/pr_ss0_scene0_int_spu_direct.cpp
    src/pr/pr_stage1_loader_spu_hal.cpp
    src/pr/pr_ss0_scene0_int_side_effect_direct.cpp
    src/pr/pr_ss0_scene0_int_load_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
    src/pr/pr_sfx.cpp
    src/pr/pr_stage1_loader_cd_hal.cpp
    src/pr/pr_stage1_loader_memory_direct.cpp
    src/audio_engine.cpp
    src/int_loader.cpp
    src/vab_player.cpp
    src/wasapi_sink.cpp
)

add_executable(test_ss0_resource_audio_direct
    src/test_ss0_resource_audio_direct.cpp
    src/pr/pr_ss0_resource_audio_direct.cpp
)
target_include_directories(test_ss0_resource_audio_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_resource_audio_direct PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
add_test(
    NAME test_ss0_resource_audio_direct
    COMMAND $<TARGET_FILE:test_ss0_resource_audio_direct>
)
target_include_directories(test_ss0_scene0_int_spu_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_scene0_int_spu_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_spu_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_spu_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_spu_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_spu_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_spu_direct"
)
add_test(
    NAME test_ss0_scene0_int_spu_direct
    COMMAND $<TARGET_FILE:test_ss0_scene0_int_spu_direct>
)

add_executable(test_ss0_scene0_int_gpu_direct
    src/test_ss0_scene0_int_gpu_direct.cpp
    src/pr/pr_ss0_scene0_int_gpu_direct.cpp
    src/pr/pr_ss0_scene0_int_renderer_direct.cpp
    src/pr/pr_ss0_scene0_int_side_effect_direct.cpp
    src/pr/pr_ss0_scene0_int_load_direct.cpp
    src/pr/pr_stage1_loader_gpu_hal.cpp
    src/pr/pr_movie_segment_direct.cpp
    src/pr/pr_stage1_loader_cd_hal.cpp
    src/pr/pr_stage1_loader_memory_direct.cpp
)
target_include_directories(test_ss0_scene0_int_gpu_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_scene0_int_gpu_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_gpu_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_gpu_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_gpu_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_gpu_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_int_gpu_direct"
)
add_test(
    NAME test_ss0_scene0_int_gpu_direct
    COMMAND $<TARGET_FILE:test_ss0_scene0_int_gpu_direct>
)

add_executable(test_ss0_scene0_shared_event_predispatch_direct
    src/test_ss0_scene0_shared_event_predispatch_direct.cpp
    src/pr/pr_ss0_scene0_shared_event_predispatch_direct.cpp
    src/pr/pr_ss0_scene0_global_bindings_direct.cpp
)
target_include_directories(
    test_ss0_scene0_shared_event_predispatch_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(
    test_ss0_scene0_shared_event_predispatch_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_shared_event_predispatch_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_shared_event_predispatch_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_shared_event_predispatch_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_shared_event_predispatch_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
        "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_shared_event_predispatch_direct"
)
add_test(
    NAME test_ss0_scene0_shared_event_predispatch_direct
    COMMAND $<TARGET_FILE:test_ss0_scene0_shared_event_predispatch_direct>
)

add_executable(test_ss0_title_entry_prefix_direct
    src/test_ss0_title_entry_prefix_direct.cpp
    src/pr/pr_ss0_title_entry_prefix_direct.cpp
)
target_include_directories(test_ss0_title_entry_prefix_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_title_entry_prefix_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_entry_prefix_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_entry_prefix_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_entry_prefix_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_entry_prefix_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_entry_prefix_direct"
)
add_test(
    NAME test_ss0_title_entry_prefix_direct
    COMMAND $<TARGET_FILE:test_ss0_title_entry_prefix_direct>
)

add_executable(test_ss0_title_initial_clock_direct
    src/test_ss0_title_initial_clock_direct.cpp
    src/pr/pr_ss0_title_initial_clock_direct.cpp
)
target_include_directories(test_ss0_title_initial_clock_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_title_initial_clock_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_initial_clock_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_initial_clock_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_initial_clock_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_initial_clock_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_initial_clock_direct"
)
add_test(
    NAME test_ss0_title_initial_clock_direct
    COMMAND $<TARGET_FILE:test_ss0_title_initial_clock_direct>
)

add_executable(test_ss0_scene0_title_result_direct
    src/test_ss0_scene0_title_result_direct.cpp
    src/pr/pr_ss0_transition_direct.cpp
)
target_include_directories(test_ss0_scene0_title_result_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_scene0_title_result_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_title_result_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_title_result_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_title_result_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_title_result_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene0_title_result_direct"
)

add_executable(test_ss0_title_graph_control
    src/test_ss0_title_graph_control.cpp
    src/pr/pr_ss0_title_transform_direct.cpp
    src/pr/pr_psx_graph_owner_direct.cpp
    src/pr/pr_psx_clear_image_direct.cpp
    src/pr/pr_psx_tmd_submit_direct.cpp
    src/pr/pr_psx_fast_sprite_submit_direct.cpp
    src/pr/pr_psx_gs_sprite_submit_direct.cpp
    src/pr/pr_psx_gte_direct.cpp
)
target_include_directories(test_ss0_title_graph_control PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_title_graph_control PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_graph_control"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_graph_control"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_graph_control"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_graph_control"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_title_graph_control"
)

add_executable(test_ss0_clear_image_direct
    src/test_ss0_clear_image_direct.cpp
    src/pr/pr_psx_clear_image_direct.cpp
)
target_include_directories(test_ss0_clear_image_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_clear_image_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_clear_image_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_clear_image_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_clear_image_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_clear_image_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_clear_image_direct"
)
add_test(
    NAME test_ss0_clear_image_direct
    COMMAND $<TARGET_FILE:test_ss0_clear_image_direct>
)

add_executable(test_ss0_dma_submit_direct
    src/test_ss0_dma_submit_direct.cpp
    src/pr/pr_psx_dma_submit_direct.cpp
)
target_include_directories(test_ss0_dma_submit_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_dma_submit_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_dma_submit_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_dma_submit_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_dma_submit_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_dma_submit_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_dma_submit_direct"
)
add_test(
    NAME test_ss0_dma_submit_direct
    COMMAND $<TARGET_FILE:test_ss0_dma_submit_direct>
)

add_executable(test_ss0_vsync_direct
    src/test_ss0_vsync_direct.cpp
    src/pr/pr_psx_vsync_direct.cpp
    src/pr/pr_psx_pad_direct.cpp
)
target_include_directories(test_ss0_vsync_direct PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(test_ss0_vsync_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_vsync_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_vsync_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_vsync_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_vsync_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_vsync_direct"
)
add_test(
    NAME test_ss0_vsync_direct
    COMMAND $<TARGET_FILE:test_ss0_vsync_direct>
)

add_executable(test_ss0_psx_gte_rtps_fifo
    src/test_ss0_psx_gte_rtps_fifo.cpp
    src/pr/pr_psx_gte_direct.cpp
)
target_include_directories(test_ss0_psx_gte_rtps_fifo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_psx_gte_rtps_fifo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_gte_rtps_fifo"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_gte_rtps_fifo"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_gte_rtps_fifo"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_gte_rtps_fifo"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_gte_rtps_fifo"
)

add_executable(test_ss0_psx_gte_mode25_triangle
    src/test_ss0_psx_gte_mode25_triangle.cpp
    src/pr/pr_psx_gte_direct.cpp
    src/pr/pr_psx_tmd_submit_direct.cpp
)
target_include_directories(test_ss0_psx_gte_mode25_triangle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_psx_gte_mode25_triangle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_gte_mode25_triangle"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_gte_mode25_triangle"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_gte_mode25_triangle"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_gte_mode25_triangle"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_gte_mode25_triangle"
)

add_executable(test_ss0_psx_graph_tmd_commit
    src/test_ss0_psx_graph_tmd_commit.cpp
    src/pr/pr_psx_graph_owner_direct.cpp
    src/pr/pr_psx_tmd_submit_direct.cpp
    src/pr/pr_psx_fast_sprite_submit_direct.cpp
    src/pr/pr_psx_gs_sprite_submit_direct.cpp
    src/pr/pr_psx_gte_direct.cpp
)
target_include_directories(test_ss0_psx_graph_tmd_commit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_psx_graph_tmd_commit PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_graph_tmd_commit"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_graph_tmd_commit"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_graph_tmd_commit"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_graph_tmd_commit"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_psx_graph_tmd_commit"
)
target_include_directories(test_ss0_memcard_backend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_memcard_backend PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_memcard_backend"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_memcard_backend"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_memcard_backend"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_memcard_backend"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_memcard_backend"
)

add_executable(test_ss0_practice_lifecycle
    src/test_ss0_practice_lifecycle.cpp
    src/pr/pr_ss0_practice_lifecycle_direct.cpp
)
target_include_directories(test_ss0_practice_lifecycle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_practice_lifecycle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_practice_lifecycle"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_practice_lifecycle"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_practice_lifecycle"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_practice_lifecycle"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_practice_lifecycle"
)

add_executable(test_ss0_directory_dispatcher
    src/test_ss0_directory_dispatcher.cpp
    src/pr/pr_ss0_directory_dispatcher_direct.cpp
)
target_include_directories(test_ss0_directory_dispatcher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_directory_dispatcher PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_directory_dispatcher"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_directory_dispatcher"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_directory_dispatcher"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_directory_dispatcher"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_directory_dispatcher"
)

add_executable(test_ss0_directory_pages_render_direct
    src/test_ss0_directory_pages_render_direct.cpp
    src/pr/pr_ss0_directory_pages_render_direct.cpp
    src/pr/pr_ss0_event_backdrop_render_direct.cpp
    src/pr/pr_psx_gs_sprite_submit_direct.cpp
    src/pr/pr_psx_gte_direct.cpp
)
target_include_directories(test_ss0_directory_pages_render_direct PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_directory_pages_render_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_directory_pages_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_directory_pages_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_directory_pages_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_directory_pages_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_directory_pages_render_direct"
)

add_executable(test_ss0_card_io_banner_render_direct
    src/test_ss0_card_io_banner_render_direct.cpp
    src/pr/pr_ss0_card_io_banner_render_direct.cpp
)
target_include_directories(test_ss0_card_io_banner_render_direct PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_card_io_banner_render_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_card_io_banner_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_card_io_banner_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_card_io_banner_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_card_io_banner_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_card_io_banner_render_direct"
)

add_executable(test_ss0_hiscore_render_direct
    src/test_ss0_hiscore_render_direct.cpp
    src/pr/pr_ss0_hiscore_render_direct.cpp
)
target_include_directories(test_ss0_hiscore_render_direct PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_hiscore_render_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_hiscore_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_hiscore_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_hiscore_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_hiscore_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_hiscore_render_direct"
)

add_executable(test_ss0_card_info_render_direct
    src/test_ss0_card_info_render_direct.cpp
    src/pr/pr_ss0_card_info_render_direct.cpp
)
target_include_directories(test_ss0_card_info_render_direct PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_card_info_render_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_card_info_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_card_info_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_card_info_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_card_info_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_card_info_render_direct"
)

add_executable(test_stage1_transition_presentation
    src/test_stage1_transition_presentation.cpp
    src/pr/pr_stage1_scene1_movie1_direct.cpp
    src/pr/pr_psx_graph_owner_direct.cpp
    src/pr/pr_psx_fast_sprite_submit_direct.cpp
    src/pr/pr_psx_gs_sprite_submit_direct.cpp
    src/pr/pr_psx_gte_direct.cpp
)
target_include_directories(test_stage1_transition_presentation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
    target_compile_options(test_stage1_transition_presentation PRIVATE /Gy)
    target_link_options(test_stage1_transition_presentation PRIVATE /OPT:REF /STACK:16777216)
endif()

add_executable(test_stage1_event4_packet_contract
    src/test_stage1_event4_packet_contract.cpp
    src/pr/pr_psx_fast_sprite_submit_direct.cpp
    src/pr/pr_ss0_event4_prompt_render_direct.cpp
)
target_include_directories(test_stage1_event4_packet_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
    target_compile_options(test_stage1_event4_packet_contract PRIVATE /Gy)
    target_link_options(test_stage1_event4_packet_contract PRIVATE /OPT:REF /STACK:16777216)
endif()

add_executable(test_ss0_event4_prompt_render_direct
    src/test_ss0_event4_prompt_render_direct.cpp
    src/pr/pr_ss0_event4_prompt_render_direct.cpp
)
target_include_directories(test_ss0_event4_prompt_render_direct PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_event4_prompt_render_direct PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_event4_prompt_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_event4_prompt_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_event4_prompt_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_event4_prompt_render_direct"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_event4_prompt_render_direct"
)

add_executable(test_ss0_load_replay_directory_authority
    src/test_ss0_load_replay_directory_authority.cpp
    src/pr/pr_ss0_card_memcard_handoff_direct.cpp
    src/pr/pr_ss0_stage_progress_bank_direct.cpp
)
target_include_directories(test_ss0_load_replay_directory_authority PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_load_replay_directory_authority PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_load_replay_directory_authority"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_load_replay_directory_authority"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_load_replay_directory_authority"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_load_replay_directory_authority"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_load_replay_directory_authority"
)

add_executable(test_ss0_scene_entry_feedback_adapter
    src/test_ss0_scene_entry_feedback_adapter.cpp
    src/pr/pr_scene_entry_card_feedback_direct.cpp
    src/pr/pr_scene_entry_feedback_adapter_direct.cpp
    src/pr/pr_scene_entry_direct.cpp
)
target_include_directories(test_ss0_scene_entry_feedback_adapter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_scene_entry_feedback_adapter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene_entry_feedback_adapter"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene_entry_feedback_adapter"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene_entry_feedback_adapter"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene_entry_feedback_adapter"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_scene_entry_feedback_adapter"
)

add_executable(test_ss0_stage_status_bank_f0_gate
    src/test_ss0_stage_status_bank_f0_gate.cpp
    src/pr/pr_stage_status_bank_direct.cpp
)
target_include_directories(test_ss0_stage_status_bank_f0_gate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage_status_bank_f0_gate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage_status_bank_f0_gate"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage_status_bank_f0_gate"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage_status_bank_f0_gate"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage_status_bank_f0_gate"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage_status_bank_f0_gate"
)

add_executable(test_ss0_stage1_clear_tail_f0_gate
    src/test_ss0_stage1_clear_tail_f0_gate.cpp
    src/pr/pr_stage1_lifecycle_direct.cpp
    src/pr/pr_stage_status_bank_direct.cpp
    src/pr/pr_stage1_movie_segment_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
    src/pr/pr_scene_entry_direct.cpp
    src/pr/pr_stage1_loader_cd_hal.cpp
)
target_include_directories(test_ss0_stage1_clear_tail_f0_gate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_clear_tail_f0_gate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_clear_tail_f0_gate"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_clear_tail_f0_gate"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_clear_tail_f0_gate"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_clear_tail_f0_gate"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_clear_tail_f0_gate"
)

add_executable(test_ss0_stage1_runner_final_return_gate
    src/test_ss0_stage1_runner_final_return_gate.cpp
    src/pr/pr_stage_runner_direct.cpp
)
target_include_directories(test_ss0_stage1_runner_final_return_gate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_runner_final_return_gate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_runner_final_return_gate"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_runner_final_return_gate"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_runner_final_return_gate"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_runner_final_return_gate"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_runner_final_return_gate"
)

add_executable(test_ss0_stage1_clear_tail_trigger_gate
    src/test_ss0_stage1_clear_tail_trigger_gate.cpp
    src/pr/pr_stage_event_direct.cpp
    src/pr/pr_stage_runner_direct.cpp
    src/pr/pr_stage1_overlay_parser.cpp
    src/pr/pr_stage1_overlay_script_text_direct.cpp
    src/int_loader.cpp
)
target_include_directories(test_ss0_stage1_clear_tail_trigger_gate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_clear_tail_trigger_gate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_clear_tail_trigger_gate"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_clear_tail_trigger_gate"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_clear_tail_trigger_gate"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_clear_tail_trigger_gate"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_clear_tail_trigger_gate"
)

add_executable(test_ss0_stage1_flag_stream_cursor_authority
    src/test_ss0_stage1_flag_stream_cursor_authority.cpp
    src/pr/pr_stage_event_direct.cpp
    src/pr/pr_stage_runner_direct.cpp
    src/pr/pr_stage1_overlay_parser.cpp
    src/pr/pr_stage1_overlay_script_text_direct.cpp
    src/int_loader.cpp
)
target_include_directories(test_ss0_stage1_flag_stream_cursor_authority PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_flag_stream_cursor_authority PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_flag_stream_cursor_authority"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_flag_stream_cursor_authority"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_flag_stream_cursor_authority"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_flag_stream_cursor_authority"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_flag_stream_cursor_authority"
)

add_executable(test_stage1_movie_matte_policy
    src/test_stage1_movie_matte_policy.cpp
)
target_include_directories(test_stage1_movie_matte_policy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(test_ss0_stage1_bucket30_row3_phase_contract
    src/test_ss0_stage1_bucket30_row3_phase_contract.cpp
    src/pr/pr_stage1_scorer_direct.cpp
)
target_include_directories(test_ss0_stage1_bucket30_row3_phase_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_bucket30_row3_phase_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_bucket30_row3_phase_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_bucket30_row3_phase_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_bucket30_row3_phase_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_bucket30_row3_phase_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_bucket30_row3_phase_contract"
)

add_executable(test_ss0_stage1_bucket30_preclear_owner_order_contract
    src/test_ss0_stage1_bucket30_preclear_owner_order_contract.cpp
    src/pr/pr_stage1_scorer_direct.cpp
)
target_include_directories(test_ss0_stage1_bucket30_preclear_owner_order_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_bucket30_preclear_owner_order_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_bucket30_preclear_owner_order_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_bucket30_preclear_owner_order_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_bucket30_preclear_owner_order_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_bucket30_preclear_owner_order_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_bucket30_preclear_owner_order_contract"
)

add_executable(test_ss0_stage1_accepted_page9_cadence_contract
    src/test_ss0_stage1_accepted_page9_cadence_contract.cpp
    src/pr/pr_stage1_scorer_direct.cpp
)
target_include_directories(test_ss0_stage1_accepted_page9_cadence_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_accepted_page9_cadence_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_accepted_page9_cadence_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_accepted_page9_cadence_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_accepted_page9_cadence_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_accepted_page9_cadence_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_accepted_page9_cadence_contract"
)

add_executable(test_ss0_stage1_14bdc_owner_set_cadence_contract
    src/test_ss0_stage1_14bdc_owner_set_cadence_contract.cpp
    src/pr/pr_stage1_scorer_direct.cpp
)
target_include_directories(test_ss0_stage1_14bdc_owner_set_cadence_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_14bdc_owner_set_cadence_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_14bdc_owner_set_cadence_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_14bdc_owner_set_cadence_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_14bdc_owner_set_cadence_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_14bdc_owner_set_cadence_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_14bdc_owner_set_cadence_contract"
)

add_executable(test_ss0_stage1_replay_mirror_observation_contract
    src/test_ss0_stage1_replay_mirror_observation_contract.cpp
    src/pr/pr_stage1_scorer_direct.cpp
)
target_include_directories(test_ss0_stage1_replay_mirror_observation_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_replay_mirror_observation_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_replay_mirror_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_replay_mirror_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_replay_mirror_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_replay_mirror_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_replay_mirror_observation_contract"
)

add_executable(test_ss0_stage1_xacd_callback_slot_observation_contract
    src/test_ss0_stage1_xacd_callback_slot_observation_contract.cpp
    src/pr/pr_stage1_xa_cd_direct.cpp
    src/pr/pr_stage1_lower_cd_producer_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
)
target_include_directories(test_ss0_stage1_xacd_callback_slot_observation_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_xacd_callback_slot_observation_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_callback_slot_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_callback_slot_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_callback_slot_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_callback_slot_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_callback_slot_observation_contract"
)

add_executable(test_ss0_stage1_xacd_status_byte_observation_contract
    src/test_ss0_stage1_xacd_status_byte_observation_contract.cpp
    src/pr/pr_stage1_xa_cd_direct.cpp
    src/pr/pr_stage1_lower_cd_producer_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
)
target_include_directories(test_ss0_stage1_xacd_status_byte_observation_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_xacd_status_byte_observation_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_status_byte_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_status_byte_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_status_byte_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_status_byte_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_status_byte_observation_contract"
)

add_executable(test_ss0_stage1_xacd_status_flags_observation_contract
    src/test_ss0_stage1_xacd_status_flags_observation_contract.cpp
    src/pr/pr_stage1_xa_cd_direct.cpp
    src/pr/pr_stage1_lower_cd_producer_direct.cpp
    src/pr/pr_stage1_loader_memory_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
)
target_include_directories(test_ss0_stage1_xacd_status_flags_observation_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_xacd_status_flags_observation_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_status_flags_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_status_flags_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_status_flags_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_status_flags_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_status_flags_observation_contract"
)

add_executable(test_ss0_stage1_xacd_interrupt_snapshot_observation_contract
    src/test_ss0_stage1_xacd_interrupt_snapshot_observation_contract.cpp
    src/pr/pr_stage1_xa_cd_direct.cpp
    src/pr/pr_stage1_lower_cd_producer_direct.cpp
    src/pr/pr_stage1_loader_memory_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
)
target_include_directories(test_ss0_stage1_xacd_interrupt_snapshot_observation_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_xacd_interrupt_snapshot_observation_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_interrupt_snapshot_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_interrupt_snapshot_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_interrupt_snapshot_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_interrupt_snapshot_observation_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_interrupt_snapshot_observation_contract"
)

add_executable(test_ss0_stage1_xacd_getloc_pending_boundary_contract
    src/test_ss0_stage1_xacd_getloc_pending_boundary_contract.cpp
    src/pr/pr_stage1_xa_cd_direct.cpp
    src/pr/pr_stage1_lower_cd_producer_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
)
target_include_directories(test_ss0_stage1_xacd_getloc_pending_boundary_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_stage1_xacd_getloc_pending_boundary_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_getloc_pending_boundary_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_getloc_pending_boundary_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_getloc_pending_boundary_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_getloc_pending_boundary_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_stage1_xacd_getloc_pending_boundary_contract"
)

add_executable(test_ss0_save_ui19148_status_bank_seed
    src/test_ss0_save_ui19148_status_bank_seed.cpp
    src/pr/pr_stage1_save_card_hal_direct.cpp
    src/pr/pr_stage1_save_ui_direct.cpp
    src/pr/pr_stage1_scorer_direct.cpp
    src/pr/pr_stage_payload_bank_direct.cpp
)
target_include_directories(test_ss0_save_ui19148_status_bank_seed PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_save_ui19148_status_bank_seed PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_save_ui19148_status_bank_seed"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_save_ui19148_status_bank_seed"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_save_ui19148_status_bank_seed"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_save_ui19148_status_bank_seed"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_save_ui19148_status_bank_seed"
)

add_executable(test_ss0_save_ui19148_directory_rows_contract
    src/test_ss0_save_ui19148_directory_rows_contract.cpp
    src/pr/pr_stage1_save_card_hal_direct.cpp
    src/pr/pr_stage1_save_ui_directory_carrier_direct.cpp
    src/pr/pr_stage1_save_ui_direct.cpp
    src/pr/pr_stage1_scorer_direct.cpp
    src/pr/pr_stage_payload_bank_direct.cpp
)
target_include_directories(test_ss0_save_ui19148_directory_rows_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_save_ui19148_directory_rows_contract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_save_ui19148_directory_rows_contract"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_save_ui19148_directory_rows_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_save_ui19148_directory_rows_contract"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_save_ui19148_directory_rows_contract"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_save_ui19148_directory_rows_contract"
)

add_executable(test_ss0_word800916f0_observation
    src/test_ss0_word800916f0_observation.cpp
    src/pr/pr_ss0_word800916f0_direct.cpp
)
target_include_directories(test_ss0_word800916f0_observation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_word800916f0_observation PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_word800916f0_observation"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_word800916f0_observation"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_word800916f0_observation"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_word800916f0_observation"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_word800916f0_observation"
)

add_executable(test_ss0_word800916f0_runtime_authority_bridge
    src/test_ss0_word800916f0_runtime_authority_bridge.cpp
    src/pr/pr_ss0_word800916f0_direct.cpp
    src/pr/pr_ss0_directory_dispatcher_direct.cpp
    src/pr/pr_stage_status_bank_direct.cpp
    src/pr/pr_stage1_lifecycle_direct.cpp
    src/pr/pr_stage1_movie_segment_direct.cpp
    src/pr/pr_movie_segment_direct.cpp
    src/pr/pr_scene_entry_direct.cpp
    src/pr/pr_stage1_loader_cd_hal.cpp
)
target_include_directories(test_ss0_word800916f0_runtime_authority_bridge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_word800916f0_runtime_authority_bridge PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_word800916f0_runtime_authority_bridge"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_word800916f0_runtime_authority_bridge"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_word800916f0_runtime_authority_bridge"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_word800916f0_runtime_authority_bridge"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_word800916f0_runtime_authority_bridge"
)

add_executable(test_ss0_state16_runtime_one_shot
    src/test_ss0_state16_runtime_one_shot.cpp
    src/pr/pr_ss0_state16_runtime_one_shot_direct.cpp
    src/pr/pr_stage1_save_card_hal_direct.cpp
    src/pr/pr_stage1_save_ui_direct.cpp
    src/pr/pr_stage1_scorer_direct.cpp
    src/pr/pr_stage_payload_bank_direct.cpp
)
target_include_directories(test_ss0_state16_runtime_one_shot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_state16_runtime_one_shot PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_state16_runtime_one_shot"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_state16_runtime_one_shot"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_state16_runtime_one_shot"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_state16_runtime_one_shot"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_state16_runtime_one_shot"
)

add_executable(test_ss0_state16_runtime_envelope_import
    src/test_ss0_state16_runtime_envelope_import.cpp
    src/pr/pr_ss0_state16_runtime_envelope_import_direct.cpp
    src/pr/pr_stage1_save_card_hal_direct.cpp
)
target_include_directories(test_ss0_state16_runtime_envelope_import PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(test_ss0_state16_runtime_envelope_import PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_state16_runtime_envelope_import"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_state16_runtime_envelope_import"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_state16_runtime_envelope_import"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_state16_runtime_envelope_import"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/test_ss0_state16_runtime_envelope_import"
)

# Shader files (copy to output)
file(GLOB SHADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.hlsl")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    configure_file(${SHADER} ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME} COPYONLY)
endforeach()
