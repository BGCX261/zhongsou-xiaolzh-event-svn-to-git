#include "word_extractor_API.h"

var_vd* get_dict_handle(var_1* _cfg_path)
{
    if (NULL == _cfg_path)
    {
        return NULL;
    }
        
    base_share_container* handle = new base_share_container;
    if (!handle)
    {
        return NULL;
    }

    var_4 ret = handle->init(_cfg_path);
    if (ret)
    {
        delete handle;
        return NULL;
    }

    return (var_vd*)handle;
}

base_word_extractor* get_extor_handle(var_1* _cfg_path, var_4 _WE_type, var_vd* _dict_handle /*= NULL*/)
{
    var_vd* share_inf = NULL;
    if (!_dict_handle)
    {
        var_vd* ret = get_dict_handle(_cfg_path);
        if (!ret)
        {
            return NULL;
        }
        share_inf = _dict_handle;
    }
    else
    {
        share_inf = _dict_handle;
    }

    base_word_extractor* cur_extor = NULL;
    switch (_WE_type)
    {
    case TF_EXTRACTOR:
        {
            cur_extor = new TF_word_extractor;
        }
        break;
    case GRAPH_EXTRACTOR:
        {
            cur_extor = new graph_word_extractor;
        }
        break;
    case CE_EXTRACTOR:
        {
            cur_extor = new TF_word_extractor;
        }
        break;
    case ZKY_EXTRACTOR:
        {
            cur_extor = new zky_word_extractor;
        }
        break;
    default:
        break;
    }

    if (cur_extor)
    {
        var_4 ret = cur_extor->init(_cfg_path, share_inf);
        if (ret)
        {
            delete cur_extor;
            cur_extor = NULL;

            return NULL;
        }
    }

    if (!cur_extor)
    {
        if (!_dict_handle)
        {
            free_dict_handle(share_inf);
        }

        return NULL;
    }

    return cur_extor;
}

var_vd free_extor_dict_handle(var_vd* _dict_handle, base_word_extractor* _extor_handle)
{
    if (_extor_handle)
    {
        delete _extor_handle;
    }

    if (_dict_handle)
    {
        delete (base_share_container*)_dict_handle;
    }
}
