set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE ALL_SOURCES LIST_DIRECTORIES true *.h *.cc)

set(SNIFFER_SOURCES config.cc main.cc file_sink.cc file_source.cc sdr.cc pss.cc sss.cc common_checks.cc dsp.cc syncer.cc phy.cc sniffer.cc ofdm.cc symbol.cc channel_mapper.cc ssb_mapper.cc worker.cc pbch.cc dmrs.cc pn_sequences.cc flow.cc rotator.cc pdcch.cc dci.cc coreset.cc bandwidth_part.cc shifter.cc flow_pool.cc proto/protocol.pb.cc)

# Add the executables
add_executable(5g_sniffer ${SNIFFER_SOURCES})
add_dependencies(5g_sniffer srsRAN)

# Create a library with all sources
add_library(${BINARY}lib STATIC ${ALL_SOURCES})

target_link_libraries(5g_sniffer srsran_phy srsran_common srsran_rf spdlog::spdlog volk liquid zmq ${Protobuf_LIBRARIES})
