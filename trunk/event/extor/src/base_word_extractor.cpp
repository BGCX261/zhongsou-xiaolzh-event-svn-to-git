#include "base_word_extractor.h"

base_word_extractor::base_word_extractor()
    : m_feature_map()
{

}

base_word_extractor::~base_word_extractor()
{

}

var_4 base_word_extractor::read_conf(UC_ReadConfigFile* _reader)
{
    memset(&m_base_conf, 0, sizeof(base_conf));
    var_4 tmp = 0;
    var_4 ret = _reader->GetFieldValue("self_cut", tmp);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_word_extractor::read_conf", "_reader->GetFieldValue", "self_cut");
        return -1;
    }
    m_base_conf._self_cut = tmp;

    ret = _reader->GetFieldValue("trunc_tit", tmp);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_word_extractor::read_conf", "_reader->GetFieldValue", "trunc_tit");
        return -1;
    }
    m_base_conf._trunc_tit = tmp;

    ret = _reader->GetFieldValue("tit_max_len", tmp);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_word_extractor::read_conf", "_reader->GetFieldValue", "tit_max_len");
        return -1;
    }
    m_base_conf._tit_max_len = tmp;

    ret = _reader->GetFieldValue("trunc_text", tmp);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_word_extractor::read_conf", "_reader->GetFieldValue", "trunc_text");
        return -1;
    }
    m_base_conf._trunc_text = tmp;

    ret = _reader->GetFieldValue("text_max_len", tmp);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_word_extractor::read_conf", "_reader->GetFieldValue", "text_max_len");
        return -1;
    }
    m_base_conf._text_max_len = tmp;

    ret = _reader->GetFieldValue("zky_data_path", m_base_conf._zky_data);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_word_extractor::read_conf", "_reader->GetFieldValue", "zky_data_path");
        return -1;
    }

    ret = _reader->GetFieldValue("tit_weight", tmp);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_word_extractor::read_conf", "_reader->GetFieldValue", "tit_weight");
        return -1;
    }
    m_base_conf._tit_weight = tmp;

    ret = _reader->GetFieldValue("feature_min_num", tmp);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_word_extractor::read_conf", "_reader->GetFieldValue", "feature_min_num");
        return -1;
    }
    //m_base_conf._feature_min_num = tmp;
    
    ret = _reader->GetFieldValue("feature_max_num", tmp);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("base_word_extractor::read_conf", "_reader->GetFieldValue", "feature_max_num");
        return -1;
    }
    m_base_conf._feature_max_num = tmp;

    return RET_SECCEED;
}

var_4 base_word_extractor::parse_news_buf( var_1* _news_buf, var_u4 _news_len )
{
    if ((16 > _news_len) || (!_news_buf))
    {
        LOG_INVALID_PARAMETER("_news_buf", "0");
        return RET_ERROR_INVALID_PARAM;
    }

    //解析指纹ID
    m_news_ID = *(var_u8*)_news_buf;
    _news_buf += 8;

    //解析标题信息
    var_4 tmp = *(var_4*)(_news_buf);
    _news_buf += 4;
    if (tmp)
    {
        m_tit_inf.left = _news_buf;
        _news_buf += tmp;
        m_tit_inf.right = _news_buf;
    }
    else
    {
        m_tit_inf.left = NULL;
        m_tit_inf.right = NULL;
    }
    
    //解析正文信息
    tmp = *(var_4*)(_news_buf);
    _news_buf += 4;
    if (tmp)
    {
        m_text_inf.left = _news_buf;
        _news_buf += tmp;
        m_text_inf.right = _news_buf;
    }
    else
    {
        m_text_inf.left = NULL;
        m_text_inf.right = NULL;
    }

    return RET_SECCEED;
}

var_4 base_word_extractor::_init(UC_ReadConfigFile* _reader)
{
    assert(_reader);
    var_4 ret = base_word_extractor::read_conf(_reader);
    if (ret)
    {
        LOG_FAILE_CALL_RET("base_word_extractor::_init", "base_word_extractor::read_conf", ret);
        return RET_ERROR_INVALID_PARAM;        
    }

    if (m_base_conf._self_cut)
    {
        if (!NLPIR_Init(m_base_conf._zky_data, 0))
        {
            LOG_FAILE_CALL_PARAM("TF_word_extractor::init", "::NLPIR_Init", m_base_conf._zky_data);
            return RET_ERROR_INVALID_PARAM;              
        }
    }

    return RET_SECCEED;
}

var_vd base_word_extractor::_uninit()
{
    if (m_base_conf._self_cut)
    {
        NLPIR_Exit();
    }
}

var_vd base_word_extractor::normalize_vector(term_info_st* _features, var_u4 _cnt)
{
    var_f4 norm = 0.f;            // 向量的模
    var_f4 normSquar = 0.f;       // 向量的模平方

    term_info_st* pos = _features;
    term_info_st* end = _features + _cnt;
    for (; end > pos; ++pos)
    {
        normSquar += (pos->weight * pos->weight);
    }

    norm = sqrt(normSquar);

    pos = _features;
    for (; end > pos; ++pos)
    {
        pos->weight /= norm;
    }
}


