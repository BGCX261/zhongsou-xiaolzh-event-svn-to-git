#ifndef __EVENT_MANAGER_H_
#define __EVENT_MANAGER_H_
#include "UH_Define.h"
#include "UC_MD5.h"
#include "UT_Queue.h"
#include "UT_HashSearch.h"
#include "UT_HashTable_Pro.h"
#include "UC_Allocator_Recycle.h"

#include "UT_Persistent_KeyValue.h"

#include "WFLog.h"
#include "WFMonitor.h"

#include "whl_list.h"

#include "word_extractor_API.h"

#include "common_def.h"
#include "utility.h"

#include "evt_mgr_confer.h"
#include "doc_infer.h"
#include "event_container.h"

namespace nsWFLog
{
    class CDailyLog;
    class CTrySaveLog;
}

namespace nsWFMonitor
{
    class CMonitor;
}

class base_word_extractor;

class UC_Allocator_Recycle;
class evt_mgr_confer;
class event_container;

class event_manager
{
public:
	event_manager();
	
    ~event_manager();

	var_4 init(
        const var_1* _cfg_path, 
        const var_4 _start_opt = DEFAULT_START
        );

	var_4 start();

    var_4 load_data();

private:
#ifdef _WIN32_ENV_
    static unsigned long __stdcall thread_req_server(void* param);
    static unsigned long __stdcall thread_del_save(void* param);
    static unsigned long __stdcall thread_upd_server(void* param);
    static unsigned long __stdcall thread_upd_extract(void* param);
    static unsigned long __stdcall thread_upd_event(void* param);
#else	
    static void* thread_req_server(void* param);	
    static void* thread_del_save(void* param);	
    static void* thread_upd_server(void* param);	
    static void* thread_upd_extract(void* param);
    static void* thread_upd_event(void* param);
#endif

    var_vd clear();

	var_4 parse_dinfer(
		var_4  _buf_len,
		var_1* _buf_ptr,
		doc_infer* _dinfer
		);

    var_4 proc_upd_files(
        const var_1* _delete_FP, 
        const var_1* _add_FP
        );

    var_4 proc_del_file( 
        const var_1* _del_FP
        );

    var_4 proc_add_file( 
        const var_1* _add_FP
        );

    var_4 recv_upd_files(
        CP_SOCKET_T _lis_sock, 
        const var_1* _delete_FP, 
        const var_1* _add_FP,
        var_4& _recv_type
        ) const;

    inline var_4 send_error_code(const CP_SOCKET_T _sock, const var_4 code)
    {
        var_1 buffer[13];
        strcpy(buffer, "EVENT1.0");
        *(var_4*)(buffer + 8) = code;
        var_4 ret = cp_sendbuf(_sock, buffer, 12);
        cp_close_socket(_sock);
        return (0 != ret) ? sys_error_code() : 0;
    }
	
    var_4 extract_key_word(
        base_word_extractor* _extor,
        doc_infer* _dinfer
        );

    var_4 load_inc(
        var_1* _inc_path
        );

#define DOC_ADD 0xABABABABABABABAB
#define DOC_DEL 0xCDCDCDCDCDCDCDCD
#define EVT_DEL 0xDEDEDEDEDEDEDEDE

    var_4 save_inc(
        doc_infer* _dpi,
        var_u8 _opt
        );

    var_4 save_bin(
        time_t _tm
        );

public:
    static evt_mgr_confer*      m_evt_confer;
	static UC_MD5				m_md5er;
private:
    nsWFLog::CDailyLog*         m_error_logger;
    nsWFLog::CDailyLog*         m_data_logger;
    nsWFLog::CTrySaveLog*       m_upd_trylogger;
	//nsWFMonitor::CMonitor*      m_moniter;          
            
    var_4                       m_run_status;
    var_u4                      m_active_thread_count;
    CP_MUTEXLOCK                m_ATC_lck;
    var_u4                      m_set_DPI_all_count;
    var_u4                      m_set_DPI_succeed_count;
    
    CP_SOCKET_T                 m_reqsvr_socket;
    CP_SOCKET_T                 m_updsvr_socket;

    var_1                       m_data_path[256];
	var_1						m_log_path[256];
    //incremental file
    FILE*                       m_file_inc;
    var_1                       m_name_sto[256];

    //FL->4M
    UC_Allocator_Recycle*       m_large_allocator;
    var_u4                      m_large_size;

    //FL->set_doc_param_info
    UC_Allocator_Recycle*       m_small_allocator;
    var_u4                      m_small_size;

    //FL->term_info_st
    UC_Allocator_Recycle*       m_term_allocator;
    var_u4                      m_term_size;

    //FL->tag_cl
    UC_Allocator_Recycle*       m_cl_allocator;
    var_u4                      m_cl_size;
    
	UC_Allocator_Recycle*		m_ar_allocator;
	var_u4						m_ar_size;

	event_container*            m_evt_ctainer;
    var_vd*                     m_dict_handle;

    UT_Queue<doc_infer*>*       m_ext_DPIs; 
    UT_Queue<doc_infer*>*       m_evt_DPIs; 

    UT_Persistent_KeyValue<var_u8>*     m_doc_storager;
};

#endif
