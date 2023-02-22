function(configure_code_analysis target_name)
	if (EUREKA_ENABLE_CODE_ANALYSIS)
	set_target_properties(
		${target_name} PROPERTIES
		VS_GLOBAL_RunCodeAnalysis false
	
		# Use visual studio core guidelines
		VS_GLOBAL_EnableMicrosoftCodeAnalysis false
		#VS_GLOBAL_CodeAnalysisRuleSet ${CMAKE_CURRENT_SOURCE_DIR}/foo.ruleset
		#VS_GLOBAL_CodeAnalysisRuleSet ${CMAKE_CURRENT_SOURCE_DIR}/foo.ruleset
	
		# Use clangtidy
		# see this for the hack to support config file https://stackoverflow.com/questions/75496453/how-to-get-visual-studio-use-a-clang-tidy-configuration-file-when-invoking-clan/75496606#75496606
		VS_GLOBAL_EnableClangTidyCodeAnalysis true
		VS_GLOBAL_ClangTidyChecks "-* \"\"--config-file=${EUREKA_CLANG_TIDY_CONFIG_PATH}" 
	)
	endif()
endfunction()











