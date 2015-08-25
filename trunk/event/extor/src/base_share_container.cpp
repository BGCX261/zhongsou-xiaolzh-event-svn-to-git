#include "base_share_container.h"

base_share_container::base_share_container()
{

}

base_share_container::~base_share_container()
{

}

//FUNC  初始化
//IN    _cfg_path: 配置文件路径
//RET   0：正常 其他：错误码
var_4 base_share_container::init(
    var_1* _cfg_path)
{
    m_status = 0;    

    UC_ReadConfigFile cur_reader;
    var_4 ret = cur_reader.InitConfigFile(_cfg_path);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_share_container::init", "cur_reader.InitConfigFile", _cfg_path);
        return RET_ERROR_INVALID_PARAM;
    }

    ret = cur_reader.GetFieldValue("base_data_path", m_TF_data);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_share_container::init", "cur_reader.GetFieldValue", "base_data_path");
        return -1;
    }

    if(cp_create_dir(m_TF_data))
    {
        LOG_FAILE_CALL_PARAM("base_share_container::init", "::cp_create_dir", m_TF_data);
        return RET_ERROR_INVALID_PARAM;
    }

    var_1 name_sto[256];
    sprintf(name_sto, "%s/NID_store.idx", m_TF_data);
    var_1 name_inc[256];
    sprintf(name_inc, "%s/NID_store.inc", m_TF_data);
    var_1 name_flg[256];
    sprintf(name_flg, "%s/NID_store.flg", m_TF_data);

    ret = m_NID_hash.init(NID_MAX_NUM / 2 + 1, NID_MAX_NUM, 0, name_sto, name_inc, name_flg);
    if (ret)
    {   
        LOG_FAILE_CALL_RET("base_share_container::init", "m_NID_hash.init", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    ret = m_idf_calcor.init(_cfg_path);
    if (ret)
    {
        LOG_FAILE_CALL_RET("base_share_container::init", "m_idf_calcor.init", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    ret = load_key_weight(&cur_reader);
    if (ret)
    {
        LOG_FAILE_CALL_RET("base_share_container::init", "base_share_container::load_key_weight", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    ret = load_stop_word(&cur_reader);
    if (ret)
    {
        LOG_FAILE_CALL_RET("base_share_container::init", "base_share_container::load_stop_word", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    SET_FLAG(m_status, WE_init);

    return RET_SECCEED;
}

var_4 base_share_container::load_key_weight(UC_ReadConfigFile* _reader)
{
    assert(_reader);
    var_1 key_file[256];
    var_4 ret = _reader->GetFieldValue("key-weight", key_file);
    if (ret)
    {
        LOG_FAILE_CALL_RET("base_share_container::load_key_weight", "_reader->GetFieldValue", ret);
        return RET_ERROR_INVALID_PARAM;
    }
    ret = m_tagweight_map.InitHashSearch(10<<10, sizeof(var_4));
    FILE* fh = fopen(key_file, "r");
    if (!fh)
    {
        LOG_FAILE_NEW(key_file);
        return RET_ERROR_INVALID_PARAM;
    }

    UC_MD5 md5er;
    var_1 buf[4<<10];
    while (fgets(buf, 4<<10, fh))
    {
        if(buf[0] == 0)
            continue;
        var_1* weight = strstr(buf, "weight=\"");
        if (!weight)
        {
            continue;
        }
        weight += 8;
        var_4 wi = atoi(weight);
        var_1* key = strstr(weight, "\">");
        if (!key)
        {
            fclose(fh);
            return RET_ERROR_INVALID_PARAM;
        }
        key += 2;
        var_1* key_end = strstr(key, "</kind>");
        if (!key_end)
        {
            fclose(fh);
            return RET_ERROR_INVALID_PARAM;
        }

        var_u8 id = md5er.MD5Bits64((var_u1*)key, key_end - key);
        ret = m_tagweight_map.AddKey_FL(id, (var_vd*)&wi);
        if (ret)
        {
            fclose(fh);
            return RET_ERROR_INVALID_PARAM;
        }
    }

    fclose(fh);
    return RET_SECCEED;
}

//FUNC  反初始化
//RET   空
var_vd base_share_container::uninit()
{

}

//FUNC  查看是否存在该newsID
//IN    _NID:newsID
//RET   1：存在 0：不存在
var_4 base_share_container::exist_nid(const var_u8 _NID)
{
    if (!TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("请先调用", "share_container::init");
        return RET_ERROR_INVALID_PARAM;    
    }

    //m_lck.lock_r();
    var_4 ret = m_NID_hash.query_key(_NID);
    //m_lck.unlock();
    if (!ret)
    {
        return 1;
    }

    return 0;
}

//FUNC  添加该newsID
//IN    _NID:newsID
//RET   0：添加成功 其他：不成功
var_4 base_share_container::add_nid(const var_u8 _NID)
{
    if (!TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("请先调用", "share_container::init");
        return RET_ERROR_INVALID_PARAM;    
    }
    
    //m_lck.lock_w();
    var_4 ret = m_NID_hash.add(_NID);
    //m_lck.unlock();
    if (0 < ret)
    {
        LOG_FAILE_CALL_RET("base_share_container::add_nid", "m_NID_hash.add", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    return RET_SECCEED;
}

var_4 base_share_container::update_DF(term_info_st** _words, var_u4 _word_cnt)
{
    if (!TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("请先调用", "share_container::init");
        return RET_ERROR_INVALID_PARAM;    
    }

    //m_lck.lock_w();
    var_4 ret = m_idf_calcor.update(_words, _word_cnt);
    //m_lck.unlock();
    if (0 < ret)
    {
        LOG_FAILE_CALL_RET("base_share_container::update_DF", "m_idf_calcor.update", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    return RET_SECCEED;
}

var_f4 base_share_container::get_IDF(var_u8 _word_id)
{
    if (!TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("请先调用", "share_container::init");
        return RET_ERROR_INVALID_PARAM;    
    }

    //m_lck.lock_w();
    var_f4 ret = m_idf_calcor.get_IDF(_word_id);
    //m_lck.unlock();
    if (0 >= ret)
    {
        LOG_FAILE_CALL_RET("base_share_container::get_IDF", "m_idf_calcor.get_IDF", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    return ret;
}

var_4 base_share_container::inc_totalDF()
{    
    if (!TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("请先调用", "share_container::init");
        return RET_ERROR_INVALID_PARAM;    
    }

//    m_lck.lock_w();
    var_4 ret = m_idf_calcor.inc_totalDF();
//    m_lck.unlock();
    if (0 < ret)
    {
        LOG_FAILE_CALL_RET("base_share_container::inc_totalDF", "m_idf_calcor.inc_totalDF", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    return RET_SECCEED;    
}

var_4 base_share_container::get_weight(var_u8 tag_id)
{
    var_4* wi = 0;
    var_4 ret = m_tagweight_map.SearchKey_FL(tag_id, (var_vd**)&wi);
    if (ret)
    {
        LOG_FAILE_CALL_RET("base_share_container::get_weight", "m_tagweight_map.SearchKey_FL", ret);
        return 1;
    }

    return *wi;
}

var_4 base_share_container::stop_word(var_1* _word)
{
    var_u8 id = m_md5er.MD5Bits64((var_u1*)_word, strlen(_word));
    var_4 ret = m_stop_hash.SearchKey_FL(id);
    if (!ret)
    {
        return 1;
    }

    return 0;
}

var_4 base_share_container::load_stop_word(UC_ReadConfigFile* _reader)
{
    assert(_reader);
    var_1 key_file[256];
    var_4 ret = _reader->GetFieldValue("stop-word", key_file);
    if (ret)
    {
        LOG_FAILE_CALL_RET("base_share_container::load_stop_word", "_reader->GetFieldValue", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    ret = m_stop_hash.InitHashSearch(1<<20);
    FILE* fh = fopen(key_file, "r");
    if (!fh)
    {
        LOG_FAILE_NEW(key_file);
        return RET_ERROR_INVALID_PARAM;
    }

    UC_MD5 md5er;
//     while (fgets(buf, 4<<10, fh))
//     {
       /* cp_drop_useless_char(buf);
		if(buf[0] == 0)
			continue;

        var_u8 id = md5er.MD5Bits64((var_u1*)buf, strlen(buf));

        ret = m_stop_hash.AddKey_FL(id);
        if (ret)
        {
            fclose(fh);
            return RET_ERROR_INVALID_PARAM;
        }*/

    var_u8 stop_id = 0u;
	while (!feof(fh))
	{		
        ret = fread(&stop_id, sizeof(var_u8), 1, fh);
		if (1 != ret)
		{
			if (!feof(fh))
			{
				fclose(fh);
				LOG_FAILE_CALL_PARAM("base_share_container::load_stop_word", "::fread", "读取停用词ID失败");
				return false;
			}
			else
				break;
		}
	
        ret = m_stop_hash.AddKey_FL(stop_id);
        if (ret<0)
        {
            fclose(fh);
            LOG_FAILE_CALL_RET("base_share_container::load_stop_word", "m_stop_hash.AddKey_FL", ret);
            return RET_ERROR_INVALID_PARAM;
        }
	}

    fclose(fh);
    return RET_SECCEED;
}
