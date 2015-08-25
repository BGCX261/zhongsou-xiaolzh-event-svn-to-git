#ifndef ___EVT_MGR_CONFER_H_
#define ___EVT_MGR_CONFER_H_

#include "UH_Define.h"
#include "UC_ReadConfigFile.h"

class evt_mgr_confer
{
public:
	var_u2		_req_port;
	var_u4		_req_out_time;
	var_u4		_req_server_count;
	var_u2		_upd_port;
	var_u4		_upd_out_time;
	var_u2		_upd_process_count;
	var_u4		_upd_queue_size;  

	var_u2		_moniter_port;
	var_u4		_moniter_out_time;

	var_u4		_max_doc_size;

	//var_1	   _doc_ID_tag[100];
	var_u4		_doc_ID_tag_len;
	var_1	  	_external_del_file[256];
	var_1	  	_external_add_file[256];
	var_1		_data_path[256];
	var_1		_log_path[256];
	var_1	   	_load_list_file[256];

	var_1	   	_extor_cfg[256];
	var_u4	  	_ext_key_num;
	var_u4		_tag_cl_num;
	var_u4		_tag_ar_num;

	evt_mgr_confer()
		: _req_port(0)
		, _req_out_time(0)
		, _req_server_count(0)
		, _upd_port(0)
		, _upd_out_time(0)
		, _upd_process_count(0)
		, _moniter_port(0)
		, _moniter_out_time(0)
		, _max_doc_size(0)
		, _load_list_file()
	{
		memset(_external_del_file, 0, sizeof(_external_del_file));
		memset(_external_add_file, 0, sizeof(_external_add_file));
		memset(_load_list_file, 0, sizeof(_load_list_file));
		memset(_data_path, 0, sizeof(_data_path));
	}

	~evt_mgr_confer()
	{
		clear();
	}

	var_4 read_cfg(UC_ReadConfigFile& _conf_reader)
	{
		clear();
		var_4 ret = _conf_reader.GetFieldValue("req_port", _req_port);
		if (0 != ret) 
		{
			return -1;
		}
		ret = _conf_reader.GetFieldValue("req_server_count", _req_server_count);
		if (0 != ret)
		{
			return -2;
		}
		ret = _conf_reader.GetFieldValue("upd_port", _upd_port);
		if (0 != ret)
		{
			return -3;
		}
		//ret = _conf_reader.GetFieldValue("upd_process_count", _upd_process_count);
		//if (0 != ret) return -4;
		_upd_process_count = 1; // 暂只支持单线程加载
		ret = _conf_reader.GetFieldValue("moniter_port", _moniter_port);
		if (0 != ret)
		{
			return -4;
		}
		ret = _conf_reader.GetFieldValue("max_doc_size", _max_doc_size);
		if (0 != ret) 
		{
			return -5;
		}
		_max_doc_size *= (1<<10);//KB->B
		ret = _conf_reader.GetFieldValue("upd_queue_size", _upd_queue_size);
		if (0 != ret) 
		{
			return -15;
		}
		ret = _conf_reader.GetFieldValue("recv_data_path", _data_path);
		if (0 != ret) 
		{
			return -16;
		}
		var_1 temp[256] = "";
		ret = _conf_reader.GetFieldValue("file_list_path", temp);
		if (0 != ret) 
		{
			return -17;
		}
		sprintf(_load_list_file, "%s/%s", temp, "load_list_file");
		ret = _conf_reader.GetFieldValue("log_path", _log_path);
		if (0 != ret)
		{
			return -18;
		}
		ret = _conf_reader.GetFieldValue("external_del_file", temp);
		if (0 != ret) 
		{
			return -12;
		}
		sprintf(_external_del_file, "%s/%s", _data_path, temp);
		ret = _conf_reader.GetFieldValue("external_add_file", temp);
		if (0 != ret) 
		{
			return -13;
		}
		sprintf(_external_add_file, "%s/%s", _data_path, temp);
		
		ret = _conf_reader.GetFieldValue("tag_cl_num", _tag_cl_num);
		if (0 != ret)
		{
			return -14;
		}
		ret = _conf_reader.GetFieldValue("tag_ar_num", _tag_ar_num);
		if (0 != ret)
		{
			return -15;
		}
		ret = _conf_reader.GetFieldValue("ext_key_num", _ext_key_num);
		if (0 != ret)
		{
			return -16;
		}
		ret = _conf_reader.GetFieldValue("req_out_time", _req_out_time);
		if (0 != ret)
		{
			return -27;
		}
		ret = _conf_reader.GetFieldValue("upd_out_time", _upd_out_time);
		if (0 != ret)
		{
			return -28;
		}
		ret = _conf_reader.GetFieldValue("moniter_out_time", _moniter_out_time);
		if (0 != ret)
		{
			return -30;
		}
		ret = _conf_reader.GetFieldValue("extor_cfg_file", _extor_cfg);
		if (0 != ret)
		{
			return -31;
		}
		return 0;
	}
	
	var_vd clear()
	{
		_req_port = 0;
		_req_server_count = 0;
		_upd_port = 0;
		_upd_process_count = 0;
		_max_doc_size = 0;
		
		memset(_data_path, 0, sizeof(_data_path));
		memset(_load_list_file, 0, sizeof(_load_list_file));
	}
};

#endif 
