#include "word_IDF_calcor.h"

word_IDF_calcor::word_IDF_calcor()
{

}

word_IDF_calcor::~word_IDF_calcor()
{

}

//FUNC  初始化
//IN    _cfg_path: 配置文件路径
//RET   0：正常 其他：错误码
var_4 word_IDF_calcor::init(
    var_1* _cfg_path)
{
    m_status = 0;

    UC_ReadConfigFile cur_reader;
    var_4 ret = cur_reader.InitConfigFile(_cfg_path);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("word_IDF_calcor::init", "cur_reader.InitConfigFile", _cfg_path);
        return RET_ERROR_INVALID_PARAM;
    }
    
    ret = cur_reader.GetFieldValue("base_data_path", m_TF_data);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("word_IDF_calcor::init", "cur_reader.GetFieldValue", "base_data_path");
        return -1;
    }

    if(cp_create_dir(m_TF_data))
    {
        LOG_FAILE_CALL_PARAM("word_IDF_calcor::init", "::cp_create_dir", m_TF_data);
        return RET_ERROR_INVALID_PARAM;
    }

    var_1 name_sto[256];
    sprintf(name_sto, "%s/IDF_store.idx", m_TF_data);
    var_1 name_inc[256];
    sprintf(name_inc, "%s/IDF_store.inc", m_TF_data);
    var_1 name_flg[256];
    sprintf(name_flg, "%s/IDF_store.flg", m_TF_data);

    ret = m_DF_hash.init( DF_MAX_NUM / 2 + 1, DF_MAX_NUM, 4, name_sto, name_inc, name_flg);
    if (ret)
    {   
        LOG_FAILE_CALL_RET("word_IDF_calcor::init", "m_DF_hash.init", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    SET_FLAG(m_status, WE_init);
    
    return 0;
}

var_vd word_IDF_calcor::uninit()
{
    
}

var_4 word_IDF_calcor::update(term_info_st** _words, var_u4 _word_cnt)
{
    if (!TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("请先调用", "TF_word_extractor::init");
        return RET_ERROR_INVALID_PARAM;    
    }

    term_info_st** pos = _words;
    term_info_st** end = pos + _word_cnt;
    var_4 one = 1;
    var_4* pre = NULL;
    for (; end > pos; ++pos)
    {
        var_4 ret = m_DF_hash.add((*pos)->word_id, &one, (var_vd**)&pre);
        if (0 > ret)
        {
            LOG_FAILE_CALL_RET("word_IDF_calcor::update", "m_DF_hash.add", ret);
            return RET_ERROR_INVALID_PARAM;
        }
        else if (1 == ret)
        {// 已存在
            assert(pre);
            ++(*pre);
        }
        else
        {
            ;
        }
    }

    return RET_SECCEED;
}

var_f4 word_IDF_calcor::get_IDF(var_u8 _word_id)
{
    var_vd* dat_key = NULL;
    var_4 ret = m_DF_hash.pop_value(_word_id, dat_key);
    if (ret)
    {
        return 1; // 如果该词没有出现过，其IDF规定为1
    }

    //! 采用lucene方法，idf的计算公式稍有不同
    var_f4 idf = (var_f4)(log(m_total_DF / (*(var_u4*)dat_key + 1.f)) + 1.f);
    m_DF_hash.push_value(dat_key);

    return idf;   
}
