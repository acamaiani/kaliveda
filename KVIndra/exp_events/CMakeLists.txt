BUILD_KALIVEDA_MODULE(exp_events
	PARENT ${KVSUBPROJECT}
	KVMOD_DEPENDS identification daq_cec geometry calibration
   LIB_EXCLUDE KVINDRAFNL KVINDRABackwardGroupReconstructor
)
