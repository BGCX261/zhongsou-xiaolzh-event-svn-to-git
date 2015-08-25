#include "event_manager.h"

UC_MD5 event_manager::m_md5er;

evt_mgr_confer* event_manager::m_evt_confer = NULL;

inline var_1* block_alloc(UC_Allocator_Recycle* _mem_pool)
{
	var_1* temp = _mem_pool->AllocMem();
	while (NULL == temp)
	{
		cp_sleep(1);
		LOG_ERROR("doc_manager::block_alloc", "memory is not enough!");
		temp = _mem_pool->AllocMem();
	}

	return temp;
}

event_manager::event_manager()
: m_error_logger(NULL)
, m_data_logger(NULL)
, m_upd_trylogger(NULL)
//, m_moniter(NULL)
, m_run_status(0)
, m_active_thread_count(0)
, m_ATC_lck()
, m_set_DPI_all_count(0)
, m_set_DPI_succeed_count(0)
, m_reqsvr_socket(INVALID_SOCKET)
, m_updsvr_socket(INVALID_SOCKET)
, m_large_allocator(NULL)
, m_large_size(0u)
, m_cl_allocator(NULL)
, m_cl_size(0u)
, m_ar_allocator(NULL)
, m_ar_size(0u)
, m_small_allocator(NULL)
, m_small_size(0u)
, m_term_allocator(NULL)
, m_term_size(0u)
, m_evt_ctainer(NULL)
, m_dict_handle(NULL)
, m_ext_DPIs(NULL)
, m_evt_DPIs(NULL)
, m_doc_storager(NULL)
{
	cp_init_socket();
}

event_manager::~event_manager()
{
	clear();
}

var_vd event_manager::clear()
{
	m_run_status = 0;
	while (0 < m_active_thread_count)
	{
		PRINT_DEBUG_INFO("event_manager::clear [%d]threads have not been terminated, waiting.....\n", m_active_thread_count);
		cp_sleep(100);
	}
	PRINT_DEBUG_INFO("event_manager::clear all threads have been terminated.\n");
	
	if (NULL != m_dict_handle)
	{
		free_dict_handle(m_dict_handle);
		m_dict_handle = NULL;
	}

	if (NULL != m_evt_DPIs)
	{
		delete m_evt_DPIs;
		m_evt_DPIs = NULL;
	}
	if (NULL != m_ext_DPIs)
	{
		delete m_ext_DPIs;
		m_ext_DPIs = NULL;
	}
	if (NULL != m_term_allocator)
	{
		delete m_term_allocator;
		m_term_allocator = NULL;
	}
	if (NULL != m_small_allocator)
	{
		delete m_large_allocator;
		m_large_allocator = NULL;
	}
	if (NULL != m_large_allocator)
	{
		delete m_large_allocator;
		m_large_allocator = NULL;
	}
	/*
	if (NULL != m_moniter)
	{
		delete m_moniter;
		m_moniter = NULL;
	}
	*/
	if (NULL != m_upd_trylogger)
	{
		delete m_upd_trylogger;
		m_upd_trylogger = NULL;
	}
	if (NULL != m_data_logger)
	{
		delete m_data_logger;
		m_data_logger = NULL;
	}
	if (NULL != m_error_logger)
	{
		delete m_error_logger;
		m_error_logger = NULL;
	}
	if (NULL != m_evt_confer)
	{
		delete m_evt_confer;
		m_evt_confer = NULL;
	}
    if (NULL != m_cl_allocator)
    {
        delete m_cl_allocator;
        m_cl_allocator = NULL;
    }
	if (NULL != m_ar_allocator)
	{
		delete m_ar_allocator;
		m_ar_allocator = NULL;
	}
}

var_4 event_manager::init(const var_1* _conf_path, const var_4 _start_opt/* = DEFAULT_START*/)
{
	var_1 s_tmp[256] = {0};
	if ((NULL != m_evt_confer) || (NULL == _conf_path))
	{
		return EMGR_ERROR_INIT - 1;
	}

	UC_ReadConfigFile conf_reader;
	var_4 ret = conf_reader.InitConfigFile(_conf_path);
	if (0 != ret)
	{
		LOG_FAILE_CALL_PARAM("event_manager::init", "conf_reader.InitConfigFile", _conf_path);
		return EMGR_ERROR_INIT - 2;
	}
	
	m_evt_confer = new evt_mgr_confer;
	if (!m_evt_confer)
	{
		LOG_FAILE_NEW("m_evt_confer");
		return EMGR_ERROR_INIT - 3;
	}
	ret = m_evt_confer->read_cfg(conf_reader);
	if (CHECK_FAILED(ret))
	{
		LOG_FAILE_CALL_RET("event_manager::init", "m_evt_infer.read_cfg", ret);
		return EMGR_ERROR_INIT - 4;
	}
	ret = conf_reader.GetFieldValue("log_path", m_log_path);
	if (ret)
	{
		LOG_FAILE_CALL_PARAM("event_manager::init", "m_evt_ctainer->init", "m_log_path");
		return RET_SECCEED;
	}
	if (_start_opt == CLEAR_DOC_START)
	{  
		sprintf(s_tmp, "%s/err_log", m_log_path);
		ClearFolder(s_tmp);
		sprintf(s_tmp, "%s/err_data", m_log_path);
		ClearFolder(s_tmp);
		sprintf(s_tmp, "%s/upd_try_log", m_log_path);
		ClearFolder(s_tmp);
		cp_remove_file(m_evt_confer->_external_del_file);
		cp_remove_file(m_evt_confer->_external_add_file);
	}

	m_evt_ctainer = new event_container;
	if (NULL == m_evt_ctainer)
	{
		LOG_FAILE_NEW("m_evt_ctainer");
		return EMGR_ERROR_INIT - 19;
	}
	m_doc_storager = new UT_Persistent_KeyValue<var_u8>;
	if (NULL == m_doc_storager)
	{
		LOG_FAILE_NEW("m_doc_storager");
		return EMGR_ERROR_INIT - 19;
	}

	var_1 doc_store_cfg[256];
	ret = conf_reader.GetFieldValue("doc_storager_cfg_path", doc_store_cfg);
	if (ret)
	{
		LOG_FAILE_CALL_PARAM("event_manager::init", "m_evt_ctainer->init", "doc_storager_cfg_path");
		return RET_SECCEED;
	}

	PRINT_DEBUG_INFO("start to resume data...\n");
	{
		CP_STAT_TIME load_time;
		load_time.set_time_begin();
		ret = m_evt_ctainer->init(&conf_reader, m_doc_storager, _start_opt);
		if (ret)
		{
			LOG_FAILE_CALL_RET("event_manager::init", "m_evt_ctainer->init", ret);
			return RET_ERROR_INVALID_PARAM;
		}
		ret = m_doc_storager->init(doc_store_cfg, _start_opt);
		if (ret)
		{
			LOG_FAILE_CALL_RET("event_manager::init", "m_doc_storager->init", ret);
			return RET_ERROR_INVALID_PARAM;
		}

		load_time.set_time_end();

		PRINT_DEBUG_INFO("finish to resume data!, cost[%.2f]ms\n", load_time.get_time_us() / 1000.f);
	}

	assert(NULL == m_error_logger);
	m_error_logger = new nsWFLog::CDailyLog;
	if (NULL == m_error_logger)
	{
		LOG_FAILE_NEW("m_error_logger");
		return EMGR_ERROR_INIT - 19;
	}
	sprintf(s_tmp, "%s/err_log", m_log_path);
	if (0 != access(s_tmp, 0))
	{
	#ifdef _WIN32_ENV_
		_mkdir(s_tmp);
	#else
		mkdir(s_tmp, S_IRWXO|S_IRWXU);
	#endif
	}

	ret = m_error_logger->Init(s_tmp, "err_log", true);
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "m_error_logger->Init", ret);
		return EMGR_ERROR_INIT - 20;
	}

	assert(NULL == m_data_logger);
	m_data_logger = new nsWFLog::CDailyLog;
	if (NULL == m_data_logger)
	{
		LOG_FAILE_NEW("m_data_logger");
		return EMGR_ERROR_INIT - 19;
	}
	
	sprintf(s_tmp,"%s/err_data", m_log_path);
	if (0 != access(s_tmp, 0))
	{
	#ifdef _WIN32_ENV_
		_mkdir(s_tmp);
	#else
		mkdir(s_tmp, S_IRWXO|S_IRWXU);
	#endif
	}
	ret = m_data_logger->Init(s_tmp, "err_data", true);
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "m_data_logger->Init", ret);
		return EMGR_ERROR_INIT - 20;
	}

	assert(NULL == m_upd_trylogger);
	m_upd_trylogger = new nsWFLog::CTrySaveLog;
	if (NULL == m_upd_trylogger)
	{
		LOG_FAILE_NEW("m_try_logger");
		return EMGR_ERROR_INIT - 24;
	}
	sprintf(s_tmp, "%s/upd_try_log", m_log_path);
	ret = m_upd_trylogger->Init(s_tmp, "try_log");
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "m_upd_trylogger->Init", ret);
		return EMGR_ERROR_INIT - 25;		
	}
	// new m_large_allocator
	assert(NULL == m_large_allocator);
	m_large_allocator = new UC_Allocator_Recycle;
	if (NULL == m_large_allocator)
	{
		LOG_FAILE_NEW("m_large_allocator");
		return EMGR_ERROR_INIT - 25;
	}
	m_large_size = 4<<20;
	var_4 count = 10000;
	m_error_logger->LPrintf(true, "m_large_allocator malloc count:[%d]\n", count);
	ret = m_large_allocator->initMem(m_large_size, count);
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "m_large_allocator->initMem", ret);
		return EMGR_ERROR_INIT - 12;
	}
	
	// new m_small_allocator
	assert(NULL == m_small_allocator);
	m_small_allocator = new UC_Allocator_Recycle;
	if (NULL == m_small_allocator)
	{
		LOG_FAILE_NEW("m_small_allocator");
		return EMGR_ERROR_INIT - 25;
	}
	m_small_size = sizeof(doc_infer);
	count = 10000;
	m_error_logger->LPrintf(true, "m_large_allocator malloc count:[%d]\n", count);
	ret = m_small_allocator->initMem(m_small_size, count);
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "m_small_allocator->initMem", ret);
		return EMGR_ERROR_INIT - 12;
	}
	//init m_cl_allocator
	assert(NULL == m_cl_allocator);
	m_cl_allocator = new UC_Allocator_Recycle;
	if (NULL == m_cl_allocator)
	{
		LOG_FAILE_NEW("m_cl_allocator");
		return EMGR_ERROR_INIT - 12;
	}
    m_cl_size = sizeof(var_u8);
    count = 10000;
    ret = m_cl_allocator->initMem(m_cl_size * m_evt_confer->_tag_cl_num, count);
    if (0 != ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "m_cl_allocator->initMem", ret);
        return EMGR_ERROR_INIT - 12;
    }
	//init m_ar_allocator
	assert(NULL == m_ar_allocator);
	m_ar_allocator = new UC_Allocator_Recycle;
	if (NULL == m_ar_allocator)
	{
		LOG_FAILE_NEW("m_ar_allocator");
		return EMGR_ERROR_INIT - 12;
	}
    m_ar_size = sizeof(var_u8);
    count = 10000;
    ret = m_ar_allocator->initMem(m_ar_size * m_evt_confer->_tag_ar_num, count);
    if (0 != ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "m_ar_allocator->initMem", ret);
        return EMGR_ERROR_INIT - 12;
    }
	// new m_term_allocator 
	assert(NULL == m_term_allocator);
	m_term_allocator = new UC_Allocator_Recycle;
	if (NULL == m_term_allocator)
	{
		LOG_FAILE_NEW("m_term_allocator");
		return EMGR_ERROR_INIT- 25;
	}
	m_term_size = sizeof(term_info_st);
	count = 10000;
	m_error_logger->LPrintf(true, "m_term_allocator malloc count:[%d]\n", count);
	ret = m_term_allocator->initMem(m_term_size * m_evt_confer->_ext_key_num, count);
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "m_term_allocator->initMem", ret);
		return EMGR_ERROR_INIT - 12;
	}

	// new m_ext_DPIs
	assert(NULL == m_ext_DPIs);
	m_ext_DPIs = new UT_Queue<doc_infer*>;
	if (NULL == m_ext_DPIs)
	{
		LOG_FAILE_NEW("m_ext_DPIs");
		return EMGR_ERROR_INIT - 7;
	}
	ret = m_ext_DPIs->InitQueue(m_evt_confer->_upd_queue_size);
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "m_ext_DPIs->InitQueue", ret);
		return EMGR_ERROR_INIT - 8;
	}
	
	// new m_evt_DPIs
	assert(NULL == m_evt_DPIs);
	m_evt_DPIs = new UT_Queue<doc_infer*>;
	if (NULL == m_ext_DPIs)
	{
		LOG_FAILE_NEW("m_evt_DPIs");
		return EMGR_ERROR_INIT - 7;
	}
	ret = m_ext_DPIs->InitQueue(m_evt_confer->_upd_queue_size);
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "m_ext_DPIs->InitQueue", ret);
		return EMGR_ERROR_INIT - 8;
	}

	m_dict_handle = get_dict_handle(m_evt_confer->_extor_cfg);
	if (!m_dict_handle)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "word_extractor_API::get_dict_handle", ret);
		return EMGR_ERROR_INIT - 8;  
	}

	m_run_status = 1;
	var_u4 idx = 0u;
	for (; m_evt_confer->_upd_process_count > idx; ++idx)
	{
		ret = cp_create_thread(thread_upd_extract, (var_vd*)this);
		if (0 != ret)
		{
			break;
		}
	}
	if (idx != m_evt_confer->_upd_process_count)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "cp_create_thread::thread_upd_extract", ret);
		return EMGR_ERROR_INIT - 23;
	}
	
	ret = cp_create_thread(thread_upd_event, (var_vd*)this);
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::init", "cp_create_thread::thread_upd_event", ret);
		return EMGR_ERROR_INIT - 23;
	}
	ret = conf_reader.GetFieldValue("recv_data_path", m_data_path);
	if (ret)
	{
		LOG_FAILE_CALL_PARAM("event_manager::init", "m_evt_ctainer->init", "recv_data_path");
		return RET_SECCEED;
	}
	sprintf(m_name_sto, "%s/evt_mgr.inc", m_data_path);
	if (0 == access(m_name_sto, 0))
	{
		if (_start_opt == CLEAR_START)
		{
			remove(m_name_sto);
		}
		else
		{
			ret = load_inc(m_name_sto);
			//!!! check m_name_sto is complete or not, if not restore it
			if (ret)
			{
				LOG_FAILE_CALL_PARAM("event_manager::init", "event_manager::load_inc", m_name_sto);
				return RET_ERROR_INVALID_PARAM;
			}
		}
	}
	
	m_file_inc = fopen(m_name_sto, "ab");
	if (NULL == m_file_inc)
	{
		LOG_FAILE_NEW("m_file_inc");
		return RET_NO_ENOUGH_MEMORY;
	}
			
	return RET_SECCEED;
}

var_4 event_manager::start()
{
	var_4 ret = 0;
	if ((0 == access(m_evt_confer->_external_del_file, 0)) &&
		(0 == access(m_evt_confer->_external_add_file, 0)))
	{
		while (1)
		{
			ret = proc_upd_files(m_evt_confer->_external_del_file, m_evt_confer->_external_add_file);
			if (!CHECK_FAILED(ret))
			{
				break;
			}
			m_error_logger->LPrintf(true, "event_manager::start proc_upd_files 失败[%d]，继续重试......\n", ret);
		}
	}

	assert(-1 == m_updsvr_socket);
	ret = cp_listen_socket(m_updsvr_socket, m_evt_confer->_upd_port);
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::start", "::cp_listen_socket", m_evt_confer->_upd_port);
		return EMGR_ERROR_INIT - 54;
	}
	ret = cp_create_thread(thread_upd_server, (var_vd*)this);
	if (CHECK_FAILED(ret))
	{
		LOG_FAILE_CALL_RET("event_manager::start", "cp_create_thread::thread_upd_server", m_evt_confer->_upd_port);
		return EMGR_ERROR_INIT - 55;
	}

	assert(-1 == m_reqsvr_socket);
	ret = cp_listen_socket(m_reqsvr_socket, m_evt_confer->_req_port);
	if (0 != ret)
	{
		LOG_FAILE_CALL_RET("event_manager::start", "::cp_listen_socket", m_evt_confer->_req_port);
		return EMGR_ERROR_INIT - 57;
	}

	var_u4 idx = 0u;
	for (; m_evt_confer->_req_server_count > idx; ++idx)
	{
		ret = cp_create_thread(thread_req_server, (var_vd*)this);
		if (0 != ret)
		{
			break;
		}
	}
	if (idx != m_evt_confer->_req_server_count)
	{
		LOG_FAILE_CALL_RET("event_manager::start", "cp_create_thread::thread_req_server", ret);
		return EMGR_ERROR_INIT - 58;
	}

	return RET_SECCEED;
}

var_4 event_manager::parse_dinfer(var_4 buf_len, var_1* buf_ptr, doc_infer* dinfer)
{
	var_4 ret = get_xml_tag(buf_ptr, buf_len, DOCID_TAG_NAME, dinfer->m_id, 1);
	if (ret)
	{
		return RET_ERROR_INVALID_PARAM;
	}
	ret = get_xml_tag(buf_ptr, buf_len, TIME_TAG_NAME,  dinfer->m_tm, 1);
	if (ret)
	{
		return RET_ERROR_INVALID_PARAM;
	}
		
//	if (get_xml_tag(buf_ptr, buf_len, EVTID_TAG_NAME, dinfer->m_eid, 1) != 0)	
//	{
//		return RET_ERROR_INVALID_PARAM - 2;
//	}
	
	var_1* buffer = block_alloc(m_large_allocator);

	// get tag cl
	dinfer->m_tag_cl.m_capacity = m_evt_confer->_tag_cl_num;
	dinfer->m_tag_cl.m_pointer = (var_u8*)block_alloc(m_cl_allocator);
	ret = get_xml_tag(buf_ptr, buf_len, CL_TAG_NAME, buffer, 1);
	if (ret > 0)
	{
		dinfer->m_tag_cl.m_capacity = m_evt_confer->_tag_cl_num;
		var_u8* pos_ptr = dinfer->m_tag_cl.m_pointer;
		var_u8* end_ptr = dinfer->m_tag_cl.m_pointer + dinfer->m_tag_cl.m_capacity;  
		buffer[ret] = 0;
		// tag cl is separated by space or semicolon
		var_1 ch = (strstr(buffer, ";") ? ';' : ' ');
		var_1* s_ptr = buffer;
		var_1* e_ptr = strchr(s_ptr, ch);
		while (e_ptr)
		{
			var_u8 word_id = m_md5er.MD5Bits64((var_u1*)s_ptr, e_ptr - s_ptr);
			ret = m_evt_ctainer->m_wid2str_map.AddKey_VL(word_id, s_ptr, e_ptr - s_ptr, NULL, NULL);
			if (ret < 0)
			{// m_wid2str add key failed
				break;
			}
			
			if (end_ptr > pos_ptr)
			{
				*pos_ptr++ = word_id;	
			}
			else
			{
				break;
			}
			s_ptr = e_ptr + 1;
			e_ptr = strchr(s_ptr, ch);
		}
		dinfer->m_tag_cl.m_size = pos_ptr - dinfer->m_tag_cl.m_pointer;
	}

	// get tag ar
	dinfer->m_tag_ar.m_capacity = m_evt_confer->_tag_ar_num;
	dinfer->m_tag_ar.m_pointer = (var_u8*)block_alloc(m_ar_allocator);
	ret = get_xml_tag(buf_ptr, buf_len, AR_TAG_NAME, buffer, 1);
	if (ret > 0)
	{
		dinfer->m_tag_ar.m_capacity = m_evt_confer->_tag_ar_num;
		var_u8* pos_ptr = dinfer->m_tag_ar.m_pointer;
		var_u8* end_ptr = dinfer->m_tag_ar.m_pointer + dinfer->m_tag_ar.m_capacity;
		buffer[ret] = 0;
		// tag ar is separated by space or semicolon
		var_1 ch = (strstr(buffer, ";") ? ';' : ' ');
		var_1* s_ptr = buffer;
		var_1* e_ptr = strchr(s_ptr, ch);
		while (e_ptr)
		{
			var_u8 word_id = m_md5er.MD5Bits64((var_u1*)s_ptr, e_ptr - s_ptr);
			ret = m_evt_ctainer->m_wid2str_map.AddKey_VL(word_id, s_ptr, e_ptr - s_ptr, NULL, NULL);
			if (ret < 0)
			{// m_wid2str_map add key failed
				break;
			}

			if (end_ptr > pos_ptr)
			{
				*(var_u8*)pos_ptr++ = word_id;
			}
			else
			{
				break;
			}
			s_ptr = e_ptr + 1;
			e_ptr = strchr(s_ptr, ch);
		}
		dinfer->m_tag_ar.m_size = pos_ptr - dinfer->m_tag_ar.m_pointer;
	}
	m_large_allocator->FreeMem(buffer);
	return RET_SECCEED;
}


var_4 event_manager::proc_upd_files( 
	const var_1* _delete_FP, 
	const var_1* _add_FP)
{
	var_4 ret = -1;
	var_4 thread_ID = nsWFThread::GetThreadID();
	//var_4 moniter_ID = m_moniter->LogIn(thread_ID, "doc_manager::thread_upd_server");
	try
	{
		m_upd_trylogger->TrySaveLPrintf("--------------------------------------\n处理删除文件:开始\n");
		ret = proc_del_file(_delete_FP);
		if (CHECK_FAILED(ret))
		{
			m_upd_trylogger->TrySaveLPrintf("处理删除文件:结束 处理del失败\n");
			m_error_logger->LPrintf(true, FAILE_CALL_RET("doc_manager::proc_upd_files", "doc_manager::proc_del_file", ret));
			throw 1;
		}
		//m_moniter->UpdateStatus(moniter_ID, thread_ID);

		m_upd_trylogger->TrySaveLPrintf("处理删除文件:结束 成功\n处理添加文件:开始\n");
		ret = proc_add_file(_add_FP);
		if (CHECK_FAILED(ret))
		{
			m_upd_trylogger->TrySaveLPrintf("处理添加文件:结束 处理add失败\n");
			m_error_logger->LPrintf(true, FAILE_CALL_RET("doc_manager::proc_upd_files", "doc_manager::proc_add_file", ret));
			throw 2;
		}
		m_upd_trylogger->TrySaveLPrintf("处理添加文件:结束 成功\n");
		//m_moniter->UpdateStatus(moniter_ID, thread_ID);
	}
	catch (const var_4 err_code)
	{
		//m_moniter->LogOut(moniter_ID, thread_ID);	  
		return err_code;
	}

	//m_moniter->LogOut(moniter_ID, thread_ID);

	while (0 != remove(_delete_FP))
	{
		m_error_logger->LPrintf(true, "failed to remove _delete_FP[%s] file!!!\n", _delete_FP);
		cp_sleep(100);
	}
	while (0 != remove(_add_FP))
	{
		m_error_logger->LPrintf(true, "failed to remove _add_FP[%s] file!!!\n", _add_FP);
		cp_sleep(100);
	}

	return RET_SECCEED;
}

var_4 event_manager::recv_upd_files(
	CP_SOCKET_T _lis_sock, 
	const var_1* _delete_FP, 
	const var_1* _add_FP,
	var_4& _recv_type) const
{
	assert(NULL != _delete_FP);
	assert(NULL != _add_FP);

	CP_SOCKET_T client_sock = -1;
	if(cp_accept_socket(_lis_sock, client_sock))
	{
		m_error_logger->LPrintf(true, FAILE_CALL("event_manager::recv_upd_files", "::cp_accept_socket"));
		return -1;
	}

	var_4 thread_ID = nsWFThread::GetThreadID();
	//var_4 moniter_ID = m_moniter->LogIn(thread_ID, "event_manager::thread_upd_server");

	cp_set_overtime(client_sock, m_evt_confer->_upd_out_time);
	var_1* buffer = block_alloc(m_large_allocator);
	assert(NULL != buffer);
	var_u4 recv_len = 0;
	var_4 ret = 0;
	try
	{
		ret = cp_recvbuf(client_sock, buffer, 16);
		if (0 != ret)
		{
			m_error_logger->LPrintf(true, FAILE_CALL_PARAM("event_manager::recv_upd_files", "::cp_recvbuf", "socket head"));
			throw -2;
		}
		_recv_type = *(var_4*)(buffer + 8);
		var_u4 data_len = *(var_u4*)(buffer + 12);
		if (0 != memcmp(buffer, "SJ_INDEX", 8))
		{
			m_error_logger->LPrintf(true, FAILE_CALL_LEN_PARAM("event_manager::recv_upd_files", "::memcmp", 8, buffer));
			throw -3;		
		}

		//m_moniter->UpdateStatus(moniter_ID, thread_ID);

		FILE* fp = NULL;
		switch(_recv_type)
		{
		case ADD_FILE:
			{
				fp = fopen(_add_FP, "wb");
			}
			break;
		case DEL_FILE:
			{
				fp = fopen(_delete_FP, "wb");			
			}
			break;
		default:
			m_error_logger->LPrintf(true, "invalid socket type!\n");
			break;
		}
		if (NULL == fp)
		{
			m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::recv_upd_files", "::fopen", _recv_type));
			throw -4;
		}
		
		var_1* data_buffer = (buffer + 16);
		while (0 < data_len)
		{
			var_u4 once_len = m_large_size - 16;
			if (data_len < once_len)
			{
				once_len = data_len;
			}
			ret = cp_recvbuf(client_sock, data_buffer, once_len);
			if (0 != ret)
			{
				m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::recv_upd_files", "::cp_recvbuf", sys_error_code()));			
				break;
			}
			//m_moniter->UpdateStatus(moniter_ID, thread_ID);

			ret = fwrite(data_buffer, once_len, 1, fp);
			if (1 != ret)
			{
				m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::recv_upd_files", "::fwrite", sys_error_code()));
				break;
			}
			//m_moniter->UpdateStatus(moniter_ID, thread_ID);

			data_len -= once_len;
			recv_len += once_len;
		}
		fclose(fp);
		if (0 != data_len) 
		{
			throw -5;
		}
		//!!! add flag file, 两者都收成功的话，生成flag文件， 处理完毕删除flag文件
		// 重新启动时，若增删文件存在而flag文件不存在，则视增删文件为不完整文件，不处理
		fp = fopen("flag", "ab");
		if (fp == NULL)
		{
			m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::recv_upd_files", "::fopen", sys_error_code()));
			throw -6;
		}
		fclose(fp);
		throw RET_SECCEED;
	}
	catch (const var_4 err_code)
	{
		//m_moniter->LogOut(moniter_ID, thread_ID);
		
		*(var_u4*)(buffer + 12) = recv_len;
		ret = cp_sendbuf(client_sock, buffer, 16);
		cp_close_socket(client_sock);
		m_large_allocator->FreeMem(buffer);
		if (0 != ret)
		{
			m_error_logger->LPrintf(true, FAILE_CALL_LEN_PARAM("event_manager::recv_upd_files", "::cp_sendbuf", 16, buffer));
			return -6;
		}
		
		return err_code;
	}
}

#ifdef _WIN32_ENV_
unsigned long __stdcall event_manager::thread_del_save(void* param)
#else	
void* event_manager::thread_del_save(void* param)
#endif
{
	event_manager* _this = static_cast<event_manager*>(param);
	
	_this->m_ATC_lck.lock();
	++_this->m_active_thread_count;
	_this->m_ATC_lck.unlock();

	var_u4 pre_time = 0u;
	var_1 dir_path[256] = "";
	var_4 ret = 0;
	while (1)
	{
		time_t _tm;
		time(&_tm);
		struct tm* p_time = localtime(&_tm);
		
		//if ((0 == p_time->tm_hour) && (5 == p_time->tm_min) && (30 > p_time->tm_sec))
		{//!!! time need to think and decide
			//if (pre_time != p_time->tm_yday)
			{
				ret = _this->save_bin(_tm);
				if (ret)
				{
					for (;;)
					{
						LOG_FAILE_CALL_RET("event_manager::thread_del_save", "event_manager::save", ret);
					}
				}

				pre_time = p_time->tm_yday;
			}
		}
		cp_sleep_s(10800);
	}
	
	_this->m_ATC_lck.lock();
	--_this->m_active_thread_count;
	_this->m_ATC_lck.unlock();

	return 0;
}

#ifdef _WIN32_ENV_
unsigned long __stdcall event_manager::thread_req_server(void* param)
#else	
void* event_manager::thread_req_server(void* param)
#endif
{
	event_manager* _this = static_cast<event_manager*>(param);

	//var_4 moniter_ID = 0;
	var_4 thread_ID = nsWFThread::GetThreadID();

	_this->m_ATC_lck.lock();
	++_this->m_active_thread_count;
	_this->m_ATC_lck.unlock();

	var_1* buffer = _this->m_large_allocator->AllocMem();
	assert(NULL != buffer);
	CP_SOCKET_T client_sock = -1;
	while (_this->m_run_status)
	{
		var_4 ret = cp_accept_socket(_this->m_reqsvr_socket, client_sock);
		if (0 != ret)
		{
			_this->m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::thread_req_server", "::cp_accept_socket", sys_error_code()));
			continue;
		}
		//moniter_ID = _this->m_moniter->LogIn(thread_ID, "event_manager::thread_req_server");

		cp_set_overtime(client_sock, _this->m_evt_confer->_req_out_time);

		ret = cp_recvbuf(client_sock, buffer, 12);
		if (0 != ret)
		{
			//_this->m_moniter->LogOut(moniter_ID, thread_ID);
			cp_close_socket(client_sock);
			_this->m_error_logger->LPrintf(true, FAILE_CALL_PARAM("event_manager::thread_req_server", "::cp_recvbuf", "socket head"));
			continue;
		}
		//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);

		if (0 != memcmp(buffer, "EVENT1.0", 8))
		{
			//_this->m_moniter->LogOut(moniter_ID, thread_ID);
			_this->send_error_code(client_sock, EMGR_ERROR_INIT - 1);
			_this->m_error_logger->LPrintf(true, FAILE_CALL_LEN_PARAM("event_manager::thread_req_server", "::memcmp", 8, buffer));
			continue;		
		}
		//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);

		var_u4 recv_len = *(var_u4*)(buffer + 8);
		if (MAX_BUFFER_SIZE < recv_len)
		{
			//_this->m_moniter->LogOut(moniter_ID, thread_ID);
			_this->send_error_code(client_sock, EMGR_ERROR_INIT - 2);
			_this->m_error_logger->LPrintf(true, "event_manager::thread_req_server, receiver buffer is beyond my cache!");
			continue;
		}
		ret = cp_recvbuf(client_sock, buffer, recv_len);
		if (0 != ret)
		{
			//_this->m_moniter->LogOut(moniter_ID, thread_ID);
			_this->send_error_code(client_sock, EMGR_ERROR_INIT - 3);
			_this->m_error_logger->LPrintf(true, FAILE_CALL_PARAM("event_manager::thread_req_server", "::cp_recvbuf", "socket data"));
			continue;
		}
		//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);
		
		//------------------------------------------处理请求------------------------------------
		//ret = _this->parse_socket(client_sock, NULL, buffer, recv_len);
		//_this->m_moniter->LogOut(moniter_ID, thread_ID);
		if (0 > ret)
		{
			_this->send_error_code(client_sock, EMGR_ERROR_INIT - 100 - ret);
			_this->m_error_logger->LPrintf(true, FAILE_CALL_PARAM("event_manager::thread_req_server", "_this->parse_socket", "socket data"));
			continue;
		}
		
	}
	_this->m_large_allocator->FreeMem(buffer);

	_this->m_ATC_lck.lock();
	--_this->m_active_thread_count;
	_this->m_ATC_lck.unlock();

	return RET_SECCEED;
}

#ifdef _WIN32_ENV_
unsigned long __stdcall event_manager::thread_upd_server(void* param)
#else	
void* event_manager::thread_upd_server(void* param)
#endif
{
	event_manager* _this = static_cast<event_manager*>(param);
	var_1* add_FP = _this->m_evt_confer->_external_add_file;
	assert('\0' != add_FP[0]);
	var_1* delete_FP = _this->m_evt_confer->_external_del_file;
	assert('\0' != delete_FP[0]);

	_this->m_ATC_lck.lock();
	++_this->m_active_thread_count;
	_this->m_ATC_lck.unlock();

	var_4 flag[2] = {-1, -1};
	var_4 recv_type = 0;
	while (_this->m_run_status)
	{
		
		var_4 ret = _this->recv_upd_files(_this->m_updsvr_socket, 
			delete_FP, add_FP, recv_type);
		if (CHECK_FAILED(ret))
		{   
			memset(flag, 0, 8);
			continue;
		}
		
		flag[recv_type] = 1;
		if ((1 == flag[ADD_FILE]) && (1 == flag[DEL_FILE]))
		{
			for (var_4 idx = 0; ; ++idx)
			{
				ret = _this->proc_upd_files(delete_FP, add_FP);
				if (0 != ret)
				{
					_this->m_error_logger->LPrintf(true, "event_manager::thread_upd_server proc_upd_files 第%d次重试，失败\n", idx);
					cp_sleep(100);
				}
				else break;
			}
			memset(flag, 0, 8);
		}
	}

	_this->m_ATC_lck.lock();
	--_this->m_active_thread_count;
	_this->m_ATC_lck.unlock();
	
	return RET_SECCEED;
}

var_4 event_manager::extract_key_word(base_word_extractor* _extor, doc_infer* _dinfer)
{
	_dinfer->m_terms.m_capacity = m_evt_confer->_ext_key_num;
	_dinfer->m_terms.m_pointer = (term_info_st*)block_alloc(m_term_allocator);

	//!!! add parse xml, and get title text and doc id 
	//资讯格式：docid(8)+titlen(4)+title(titlen)+doclen(4)+doc(doclen)
	//var_4 ret = _extor->extract(*(var_4*)_dinfer->m_buf_ptr, _dinfer->m_buf_ptr + 4);
	var_1* buffer = block_alloc(m_large_allocator);
	var_4 ret, buf_len= 0;
	try	
	{
		ret = get_xml_tag(_dinfer->m_buf_ptr + 4, *(var_4*)_dinfer->m_buf_ptr, DOCID_TAG_NAME, *(var_u8*)buffer, 1);
		if (ret < 0)
		{
			LOG_FAILE_CALL_RET("event_manager::extract_key_word", "get doc id", ret);
			throw RET_ERROR_INVALID_PARAM;
		}
		buf_len += 8;
		ret = get_xml_tag(_dinfer->m_buf_ptr + 4, *(var_4*)_dinfer->m_buf_ptr, TITLE_TAG_NAME, buffer + 12, 1);
		if (ret <0)
		{
			LOG_FAILE_CALL_RET("event_manager::extract_key_word", "get title", ret);
			throw RET_ERROR_INVALID_PARAM;
		}
		*(var_4*)(buffer + buf_len) = ret;
		buf_len += ret + 4;
		
		ret = get_xml_tag(_dinfer->m_buf_ptr + 4, *(var_4*)_dinfer->m_buf_ptr, TEXT_TAG_NAME, buffer + buf_len, 1);
		if(ret < 0)
		{
			LOG_FAILE_CALL_RET("event_mangager::extract_key_word", "get text", ret);
			throw RET_ERROR_INVALID_PARAM;
		}
		*(var_4*)(buffer + buf_len) = ret;
		buf_len += ret + 4;

		ret = _extor->extract(buf_len, buffer);
		if (ret)
		{
			LOG_FAILE_CALL_RET("event_manager::extract_key_word", "_extor->extract", ret);
			throw RET_ERROR_INVALID_PARAM;
		}

		ret = _extor->get_term(WT_ALL, _dinfer->m_terms.m_pointer, _dinfer->m_terms.m_capacity);
		if (ret)
		{
			LOG_FAILE_CALL_RET("event_manager::extract_key_word", "_extor->get_term", ret);
			throw RET_ERROR_INVALID_PARAM;
		}

		_dinfer->m_terms.m_size = ret;
		
		simple_pair<var_1*, var_4> word_info;
		var_4 idx = 0;
		for(; idx < _dinfer->m_terms.m_size; ++idx)
		{
			ret = m_evt_ctainer->m_wid2str_map.AddKey_VL(
							_dinfer->m_terms.m_pointer[idx].word_id,
							_dinfer->m_terms.m_pointer[idx].word_str,
							strlen(_dinfer->m_terms.m_pointer[idx].word_str),NULL,NULL);
			if (ret < 0)
			{
				LOG_FAILE_CALL_RET("event_manager::extract_key_word", "m_wid2str_map.AddKey_VL", ret);
				throw RET_ERROR_INVALID_PARAM;
			}
		}
		m_large_allocator->FreeMem(buffer);
	}
	catch (const var_4 _err_code)
	{
		m_term_allocator->FreeMem((var_1*)_dinfer->m_terms.m_pointer);
		m_large_allocator->FreeMem(buffer);
		return _err_code;
	}
	return RET_SECCEED;
}

#ifdef _WIN32_ENV_
unsigned long __stdcall event_manager::thread_upd_extract(void* param)
#else	
void* event_manager::thread_upd_extract(void* param)
#endif
{
	event_manager* _this = static_cast<event_manager*>(param);
	assert(_this->m_dict_handle);

	base_word_extractor* cur_extor = get_extor_handle(m_evt_confer->_extor_cfg, TF_EXTRACTOR, _this->m_dict_handle);
	if (!cur_extor)
	{
		LOG_FAILE_CALL_PARAM("event_manager::thread_upd_extract", "word_extractor_API::get_extor_handle", _this->m_evt_confer->_extor_cfg);
		assert(0);
	}

	_this->m_ATC_lck.lock();
	++_this->m_active_thread_count;
	_this->m_ATC_lck.unlock();

	var_4 thread_ID = nsWFThread::GetThreadID();
	//var_4 moniter_ID = _this->m_moniter->LogIn(thread_ID, "event_manager::thread_upd_extract");

	doc_infer* cur_param = NULL;
	var_4 ret = 0;
	var_u4 merge_count = 0u;
	var_u4 new_count = 0u;
	while (_this->m_run_status)
	{
		ret = _this->m_ext_DPIs->PopData_NB(cur_param);	
		if (0 != ret)
		{
		    //_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);
			cp_sleep(10);
			continue;
		}
		//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);

		ret = _this->m_doc_storager->add(cur_param->m_id, 
			cur_param->m_buf_ptr + 4, 
			*(var_4*)cur_param->m_buf_ptr);
		if (ret)
		{// 失败
			_this->m_ATC_lck.lock();
			++_this->m_set_DPI_all_count;
			_this->m_ATC_lck.unlock();

			_this->m_large_allocator->FreeMem(cur_param->m_buf_ptr);
			_this->m_small_allocator->FreeMem((var_1*)cur_param);
			//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);
			LOG_FAILE_CALL_RET("event_manager::thread_upd_extract", "m_doc_storager->add", ret);
			continue;
		}

		ret = _this->extract_key_word(cur_extor, cur_param);
		if (ret)
		{// 失败
			_this->m_ATC_lck.lock();
			++_this->m_set_DPI_all_count;
			_this->m_ATC_lck.unlock();

			_this->m_large_allocator->FreeMem(cur_param->m_buf_ptr);
			_this->m_small_allocator->FreeMem((var_1*)cur_param);
			//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);
			LOG_FAILE_CALL_RET("event_manager::thread_upd_extract", "_this->extract_key_word", ret);
			continue;
		}

		ret = _this->save_inc(cur_param, DOC_ADD);
		if (ret)
		{// 失败
			for (;;)
			{
				LOG_FAILE_CALL_RET("event_manager::thread_upd_extract", "_this->save_inc", ret);
			}
		}

		//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);
		
		while (0 != _this->m_evt_DPIs->PushData_NB(cur_param))
		{
		//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);
			cp_sleep(10);
		}		
	}

	//_this->m_moniter->LogOut(moniter_ID, thread_ID);

	free_extor_handle(cur_extor);

	_this->m_ATC_lck.lock();
	--_this->m_active_thread_count;
	_this->m_ATC_lck.unlock();

	return RET_SECCEED;
}

#ifdef _WIN32_ENV_
unsigned long __stdcall event_manager::thread_upd_event(void* param)
#else	
void* event_manager::thread_upd_event(void* param)
#endif
{
	event_manager* _this = static_cast<event_manager*>(param);
	
	_this->m_ATC_lck.lock();
	++_this->m_active_thread_count;
	_this->m_ATC_lck.unlock();

	var_4 thread_ID = nsWFThread::GetThreadID();
	//var_4 moniter_ID = _this->m_moniter->LogIn(thread_ID, "event_manager::thread_upd_event");

	doc_infer* cur_param = NULL;
	var_4 ret = 0;
	var_u4 merge_count = 0u;
	var_u4 new_count = 0u;
	while (_this->m_run_status)
	{
		ret = _this->m_evt_DPIs->PopData_NB(cur_param);	
		if (0 != ret)
		{
			//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);
			cp_sleep(1);
			continue;
		}
		//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);

		ret = _this->m_evt_ctainer->add_doc(cur_param);
		if (!ret)
		{
			_this->m_ATC_lck.lock();
			++_this->m_set_DPI_succeed_count;
			++_this->m_set_DPI_all_count;
			_this->m_ATC_lck.unlock();
		}
		else
		{
			_this->m_ATC_lck.lock();
			++_this->m_set_DPI_all_count;
			_this->m_ATC_lck.unlock();
		}
		//_this->m_moniter->UpdateStatus(moniter_ID, thread_ID);
		
		_this->m_cl_allocator->FreeMem((var_1*)cur_param->m_tag_cl.m_pointer);
		_this->m_ar_allocator->FreeMem((var_1*)cur_param->m_tag_ar.m_pointer);
		_this->m_term_allocator->FreeMem((var_1*)cur_param->m_terms.m_pointer);
		_this->m_large_allocator->FreeMem((var_1*)cur_param->m_buf_ptr);
		_this->m_small_allocator->FreeMem((var_1*)cur_param);
	}

	//_this->m_moniter->LogOut(moniter_ID, thread_ID);

	_this->m_ATC_lck.lock();
	--_this->m_active_thread_count;
	_this->m_ATC_lck.unlock();

	return RET_SECCEED;
}

var_4 event_manager::proc_del_file(const var_1* _del_FP)
{
	var_1 buf_id[64] = "";
	var_u8 doc_id = 0u;
	var_4 ret =0;
	FILE* fp = fopen(_del_FP, "rb");
	if(NULL == fp)
	{
		m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::proc_del_file", "::fopen", sys_error_code()));
		return RET_ERROR_INVALID_PARAM;
	}
	ret = fread(buf_id, 4, 1, fp);
	var_4 real_del = 0;
	var_4 all_del = 0;
	if (1 == ret)
	{
		while(fgets(buf_id, 64, fp))
		{
			cp_drop_useless_char(buf_id);
			if(buf_id[0] == 0)
				continue;
			++all_del;
			doc_id = cp_strtoval_u64(buf_id);
			ret = m_evt_ctainer->del_doc(doc_id);
			if (ret)
			{
				m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::proc_del_file", "event_manager::delete_DIs", ret));
				continue;
			}
			++real_del;
		}
	}

	fclose(fp);
	
	m_error_logger->LPrintf(true, "event_manager::proc_del_file 期望删除%d个Doc，成功删除%d个Doc!\n", all_del, real_del);
	
	return real_del;
}

var_4 event_manager::proc_add_file(const var_1* _add_FP)
{
	FILE* fp = fopen(_add_FP, "rb");
	if(NULL == fp)
	{
		m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::proc_add_file", "::fopen", sys_error_code()));
		return RET_ERROR_INVALID_PARAM;
	}

	CP_STAT_TIME stat_time;
	stat_time.set_time_begin();	
	m_error_logger->LPrintf(true, "event_manager::proc_add_file, start to load [%s] file, please waiting...\n", _add_FP);

	var_4 read_len = 0;   
	m_set_DPI_all_count = 0;
	m_set_DPI_succeed_count = 0;
	var_4 all_add = 0;
	while (1)
	{
		var_4 ret = fread(&read_len, 4, 1, fp);
		if (1 != ret)
		{
			break;
		}
		++all_add;
		var_1* buffer = block_alloc(m_large_allocator);
		if (read_len > (m_large_size - 4))
		{
			var_4 temp = min(read_len, m_large_size);
			m_error_logger->LPrintf(true, "new doc[%d] is beyond the max buffer size[%d]\n", read_len, m_large_size);
			ret = fread(buffer, temp, 1, fp);
			if (1 != ret)
			{
				m_data_logger->LPrintf(false, "数据过长，[%d]超过了Buffer限制[%d]\n错误数据\n[读取失败]\n", read_len, m_large_size - 4);
				m_large_allocator->FreeMem(buffer);
				break;
			}
			else
			{
				m_data_logger->LPrintf(false, "数据过长，[%d]超过了Buffer限制[%d]\n错误数据\n[%.*s]\n\n\n", read_len, m_large_size - 4, temp, buffer);
				fseek(fp, read_len - temp, SEEK_CUR);
				m_large_allocator->FreeMem(buffer);
				continue;
			}
		}
		
		ret = fread(buffer + 4, read_len, 1, fp);
		if (1 != ret)
		{
			m_large_allocator->FreeMem(buffer);
			m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::proc_add_file", "::fread", sys_error_code()));
			break;
		}
		if (read_len > m_evt_confer->_max_doc_size)
		{
			m_data_logger->LPrintf(false, "数据过长，[%d]超过了配置限制[%d]\n错误数据\n[%.*s]\n\n\n", read_len, m_evt_confer->_max_doc_size,
				read_len, buffer + 4);
			m_large_allocator->FreeMem(buffer);
			m_error_logger->LPrintf(true, "new doc[%d] is beyond the max doc size[%d]\n", read_len, m_evt_confer->_max_doc_size);
			//fseek(fp, read_len, SEEK_CUR);
			continue;
		}
		doc_infer* cur_param = (doc_infer*)block_alloc(m_small_allocator);
		assert(NULL != cur_param);
		//
		ret = parse_dinfer(read_len, buffer + 4, cur_param);
		if (ret)
		{
			m_cl_allocator->FreeMem((var_1*)cur_param->m_tag_cl.m_pointer);
			m_ar_allocator->FreeMem((var_1*)cur_param->m_tag_ar.m_pointer);
			m_small_allocator->FreeMem((var_1*)cur_param);
			m_large_allocator->FreeMem(buffer);
			m_error_logger->LPrintf(true, "event_manager::proc_add_file, invalid doc\n");
			continue;
		}
		//	
		*(var_4*)buffer = read_len;
		cur_param->m_buf_ptr = buffer;
		
		cur_param->m_terms.reset();

		while (0 != m_ext_DPIs->PushData_NB(cur_param))
		{
			cp_sleep(1);
		}
		if (0 == (all_add% 10000))
		{
			PRINT_DEBUG_INFO("event_manager::proc_add_file, 已成功读取%d条Doc\n", all_add);
		}
	}
	fclose(fp);
	while (m_set_DPI_all_count != all_add)
	{
		PRINT_DEBUG_INFO("event_manager::proc_add_file, 已全部读取, 等待分析...\n");
		cp_sleep(100);
	}
	stat_time.set_time_end();

	m_error_logger->LPrintf(true, "event_manager::proc_add_file 成功加载[%s]文件，耗时[%.3f]秒\n期望添加%d个Doc，成功添加%d个Doc!\n", 
		_add_FP, stat_time.get_time_ms() / 1000.f, all_add, m_set_DPI_succeed_count);
	
	m_set_DPI_succeed_count = 0;

	return RET_SECCEED;
}

var_4 event_manager::load_data()
{
	if ((0 == m_evt_confer->_load_list_file[0]) || (0 != access(m_evt_confer->_load_list_file, 0)))
	{
		return RET_FALSE;
	}
	FILE* fh = fopen(m_evt_confer->_load_list_file, "r");
	if (NULL == fh)
	{
		m_error_logger->LPrintf(true, FAILE_NEW(m_evt_confer->_load_list_file));
		return RET_ERROR_INVALID_PARAM;
	}

	var_1 add_file[256] = "";
	while(fgets(add_file, 256, fh))
	{
		cp_drop_useless_char(add_file);
		if(add_file[0] == 0)
			continue;

		var_4 ret = proc_add_file(add_file);
		if (0 > ret)
		{
			m_error_logger->LPrintf(true, "event_manager::load_data, failed to load [%s] file, wait dispose\n", add_file);
			cp_wait_dispose();
		}
	}

	fclose(fh);
	return RET_SECCEED;
}

var_4 event_manager::load_inc(var_1* _inc_path)
{
	FILE* fp = fopen(_inc_path, "rb");
	if(NULL == fp)
	{
		m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::load_inc", "::fopen", sys_error_code()));
		return RET_ERROR_INVALID_PARAM;
	}

	CP_STAT_TIME stat_time;
	stat_time.set_time_begin();	
	m_error_logger->LPrintf(true, "event_manager::load_inc, start to load [%s] file, please waiting...\n", _inc_path);

	var_4 read_len = 0;   
	m_set_DPI_all_count = 0;
	m_set_DPI_succeed_count = 0;
	var_u4 del_all_cnt = 0;
	var_u4 del_real_cnt = 0;
	var_4 all_add = 0;
	var_u8 opt = 0;
	var_u8 eid = 0;
	while (1)
	{
		var_4 ret = fread(&opt, 8, 1, fp);
		if (1 != ret)
		{
			break;
		}
		
		if (opt == EVT_DEL)
		{
			ret = fread(&eid, 8, 1, fp);
			if (1 != ret)
			{
				LOG_FAILE_CALL_PARAM("event_manager::load_inc", "::fread", "读取待删时间ID失败");
				break;
			}
			++del_all_cnt;

			while (m_set_DPI_all_count != all_add)
			{
				PRINT_DEBUG_INFO("等待添加文件处理完毕...\n");
				cp_sleep(100);
			}

			ret = m_evt_ctainer->del_evt(eid);
			if (ret)
			{
				FAILE_CALL_ID("event_manager::load_inc", "m_evt_ctainer->del_evt", eid);				
			}
			else
			{
				++del_real_cnt;
			}
			continue;
		}
		else if (opt == DOC_DEL)
		{
			;
		}
		else if (opt != DOC_ADD)
		{
			assert(0);
		}

		ret = fread(&read_len, 4, 1, fp);
		if (1 != ret)
		{
			break;
		}
		++all_add;
		var_1* buffer = block_alloc(m_large_allocator);
		if (read_len > (m_large_size - 4))
		{
			var_4 temp = min(read_len, m_large_size);
			m_error_logger->LPrintf(true, "new doc[%d] is beyond the max buffer size[%d]\n", read_len, m_large_size);
			ret = fread(buffer, temp, 1, fp);
			if (1 != ret)
			{
				m_data_logger->LPrintf(false, "数据过长，[%d]超过了Buffer限制[%d]\n错误数据\n[读取失败]\n", read_len, m_large_size - 4);
				m_large_allocator->FreeMem(buffer);
				break;
			}
			else
			{
				m_data_logger->LPrintf(false, "数据过长，[%d]超过了Buffer限制[%d]\n错误数据\n[%.*s]\n\n\n", read_len, m_large_size - 4, temp, buffer);
				fseek(fp, read_len - temp, SEEK_CUR);
				m_large_allocator->FreeMem(buffer);
				continue;
			}
		}
		
		ret = fread(buffer + 4, read_len, 1, fp);
		if (1 != ret)
		{
			m_large_allocator->FreeMem(buffer);
			m_error_logger->LPrintf(true, FAILE_CALL_RET("event_manager::proc_add_file", "::fread", sys_error_code()));
			break;
		}
		if (read_len > m_evt_confer->_max_doc_size)
		{
			m_data_logger->LPrintf(false, "数据过长，[%d]超过了配置限制[%d]\n错误数据\n[%.*s]\n\n\n", read_len, m_evt_confer->_max_doc_size,
				read_len, buffer + 4);
			m_large_allocator->FreeMem(buffer);
			m_error_logger->LPrintf(true, "new doc[%d] is beyond the max doc size[%d]\n", read_len, m_evt_confer->_max_doc_size);
			//fseek(fp, read_len, SEEK_CUR);
			continue;
		}
		doc_infer* cur_param = (doc_infer*)block_alloc(m_small_allocator);
		assert(NULL != cur_param);
//////////////////		
		if (parse_dinfer(read_len, buffer + 4, cur_param) != 0)	
		{
			m_small_allocator->FreeMem((var_1*)cur_param);
			m_large_allocator->FreeMem(buffer);
			m_error_logger->LPrintf(true, "event_manager::proc_add_file, invalid doc\n");
			continue;
		}
//////////////////

		*(var_4*)buffer = read_len;
		cur_param->m_buf_ptr = buffer;
		
		cur_param->m_terms.reset();

		while (0 != m_ext_DPIs->PushData_NB(cur_param))
		{
			cp_sleep(1);
		}
		if (0 == (all_add% 10000))
		{
			PRINT_DEBUG_INFO("event_manager::proc_add_file, 已成功读取%d条Doc\n", all_add);
		}
	}
	fclose(fp);
	while (m_set_DPI_all_count != all_add)
	{
		PRINT_DEBUG_INFO("event_manager::proc_add_file, 已全部读取, 等待分析...\n");
		cp_sleep(100);
	}
	stat_time.set_time_end();

	m_error_logger->LPrintf(true, "event_manager::proc_add_file 成功加载[%s]文件，耗时[%.3f]秒\n"
		"期望添加%d个Doc，成功添加%d个Doc，期望删除%d个事件，成功删除%d个事件!\n", 
		_inc_path, stat_time.get_time_ms() / 1000.f, all_add, m_set_DPI_succeed_count, del_all_cnt, del_real_cnt);
	
	m_set_DPI_succeed_count = 0;

	return RET_SECCEED;
}

var_4 event_manager::save_inc(doc_infer* _dpi, var_u8 _opt)
{
	// 添加锁，多线程调用
	assert(m_file_inc);
	var_4 pre_size = ftell(m_file_inc);
	var_4 ret = 0;
	try
	{
		var_4 ret = fwrite(&_opt, 8, 1, m_file_inc);
		if (1 != ret)
		{
			LOG_FAILE_CALL_RET("event_manager::save_inc", "::fwrite", ret);
			throw RET_ERROR_INVALID_PARAM;
		}

		if (DOC_ADD == _opt)
		{
			ret = fwrite(_dpi->m_buf_ptr, *(var_4*)_dpi->m_buf_ptr + 4, 1, m_file_inc);
			if (1 != ret)
			{
				LOG_FAILE_CALL_RET("event_manager::save_inc", "::fwrite", ret);
				throw RET_ERROR_INVALID_PARAM;
			}
		}
		else if (EVT_DEL == _opt || DOC_DEL == _opt)
		{
			ret = fwrite(&_dpi->m_id, 8, 1, m_file_inc);
			if (1 != ret)
			{
				LOG_FAILE_CALL_RET("event_manager::save_inc", "::fwrite", ret);
				throw RET_ERROR_INVALID_PARAM;
			}
		}
		else
		{
			assert(0);
		}
	}
	catch (const var_4 _err_code)
	{
		ret = cp_change_file_size(m_file_inc, pre_size);	
		if (ret)
		{
			LOG_FAILE_CALL_RET("event_manager::save_inc", "::cp_change_file_size", ret);
			return RET_ERROR_INVALID_PARAM;
		}
		return _err_code;
	}

	fflush(m_file_inc);
	return RET_SECCEED;
}

var_4 event_manager::save_bin(time_t _tm)
{
	var_1 dt_buffer[256] = "";
	var_1 s_tmp[256];
	nsWFPub::GetDateTimeStr(dt_buffer,_tm);
	sprintf(s_tmp, "%s/data_save.log", m_log_path);
	FILE* fp = fopen(s_tmp, "a");
	while(NULL == fp)
	{
		PRINT_DEBUG_INFO("failed to fopen data_save.log\n");
		fp = fopen(s_tmp, "a");
	}

	fprintf(fp, "%s: 开始淘汰旧结点、落地数据......\n", dt_buffer);

	CP_STAT_TIME stat_time;
	stat_time.set_time_begin();	

	var_4 ret = 0;

	do{
		ret = m_evt_ctainer->save();
		if (ret)
		{
			LOG_FAILE_CALL_RET("event_manager::thread_del_save", "m_evt_ctainer->save", ret);							
		}

	} while (ret);

	stat_time.set_time_end();
	fprintf(fp, "此次落地总耗时：%.2f 毫秒\n\n", stat_time.get_time_ms() / 1000.f );
	fclose(fp);

	return RET_SECCEED;
}

enum ErrorType
{
	TYPE_OK = 0,	//运行正常
	TYPE_MONITOR,   //monitor错误
	TYPE_NETWORK,   //网络故障
	TYPE_SERVICE,   //服务错误
	TYPE_OTHER,	 //其它错误
};
	
enum ErrorLevel
{
	LEVEL_A = 1,	//严重
	LEVEL_B,		//重要
	LEVEL_C,		//一般
	LEVEL_D,		//调试
	LEVEL_E,		//可忽略
};

int main(int argc, char* argv[])
{
	var_1* cfg_path = "./event_manager.cfg";
	var_4 start_opt = DEFAULT_START;
	if ((argc > 1) && (0 == strcmp(argv[1], "init")))
	{
		start_opt = CLEAR_DOC_START;
	}

	event_manager dmgr;
	var_4 ret = dmgr.init(cfg_path, start_opt);
	if (CHECK_FAILED(ret))
	{
		LOG_FAILE_CALL_PARAM("::main", "INDEX::event_manager::init", cfg_path);
		return -1;
	}

	if (CLEAR_DOC_START == start_opt)
	{
		 ret = dmgr.load_data();
		 if (CHECK_FAILED(ret))
		 {
			 LOG_FAILE_CALL_RET("event_manager::start_up", "event_manager::load_data", ret);
			 return ret;
		 }

		return 0;
	}
	
	ret = dmgr.start();
	if (CHECK_FAILED(ret))
	{
		LOG_FAILE_CALL_PARAM("::main", "INDEX::event_manager::start_up", cfg_path);
		return -1;
	}

	PRINT_DEBUG_INFO("event_manager starts successfully......\n");

	CP_SOCKET_T lis_sock;
	ret = cp_listen_socket(lis_sock, dmgr.m_evt_confer->_moniter_port);
	if (0 != ret)
	{
		assert(0);
	}

	var_1 moniter_buffer[1024];
	nsWFLog::CDailyLog _logger;
	ret = _logger.Init("./", "monitor");
	if (0 != ret)
	{
		assert(0);
	}
	while (1)
	{
		CP_SOCKET_T sock;
		ret = cp_accept_socket(lis_sock, sock);
		if (0 != ret)
		{
			_logger.LPrintf(false, "main: failed to accept\n");
			continue;
		}
		_logger.LPrintf(false, "main: succeed to accept[%d]\n", sock);

		cp_set_overtime(sock, 5000);

		ret = cp_recvbuf(sock, moniter_buffer, 4);
		if (0 != ret)
		{
			_logger.LPrintf(false, "main: failed to accept error code[%d]\n", sys_error_code());
			cp_close_socket(sock);
			continue;	
		}

		var_1* pos = moniter_buffer;
		memcpy(pos, "MonitorP1 ", 10);
		pos += 10 + 4;
		*(var_4*)pos = TYPE_OK;
		pos += 4;
		*(var_4*)pos = LEVEL_B;
		pos += 4;

		*(var_4*)(moniter_buffer + 10) = pos - moniter_buffer - 14;
		ret = cp_sendbuf(sock, moniter_buffer, pos - moniter_buffer);
		cp_close_socket(sock);
		if (0 != ret)
		{
			_logger.LPrintf(false, "main: failed to accept error code[%d]\n", sys_error_code());
			continue;	
		}
	}
	
	return 0;
}

