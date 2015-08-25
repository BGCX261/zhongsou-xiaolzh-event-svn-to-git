#include "doc_infer.h"

doc_infer::doc_infer()
{

}

doc_infer::~doc_infer()
{

}

var_4 doc_infer::deserialize(FILE* _fh)
{
	var_4 ret = fread(&m_tm, sizeof(m_tm), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}

	ret = fread(&m_id, sizeof(m_id), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	
	ret = fread(&m_eid, sizeof(m_eid), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	//
	ret = fread(&m_tag_cl.m_size, sizeof(m_tag_cl.m_size), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	ret = fread(m_tag_cl.m_pointer, sizeof(var_u8), m_tag_cl.m_size, _fh);
	if (ret != m_tag_cl.m_size)
	{
		return RET_FALSE;
	}
	//
	ret = fread(&m_tag_ar.m_size, sizeof(m_tag_ar.m_size), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	ret = fread(m_tag_ar.m_pointer, sizeof(var_u8), m_tag_ar.m_size, _fh);
	if (ret != m_tag_ar.m_size)
	{
		return RET_FALSE;
	}
	//
	ret = fread(&m_terms.m_size, sizeof(m_terms.m_size), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	ret = fread(m_terms.m_pointer, sizeof(term_info_st), m_terms.m_size, _fh);
	if (ret != m_terms.m_size)
	{
		return RET_FALSE;
	}
	return RET_SECCEED;
}


var_4 doc_infer::serialize(FILE* _fh)
{
	var_4 ret = fwrite(&m_tm, sizeof(m_tm), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}

	ret = fwrite(&m_id, sizeof(m_id), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}

	ret = fwrite(&m_eid, sizeof(m_eid), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	// m_buf_ptr²»ÂäµØ
	//
	ret = fwrite(&m_tag_cl.m_size, sizeof(m_tag_cl.m_size), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	ret = fwrite(m_tag_cl.m_pointer, sizeof(var_u8), m_tag_cl.m_size, _fh);
	if (ret != m_tag_cl.m_size)
	{
		return RET_FALSE;
	}
	//
	ret = fwrite(&m_tag_ar.m_size, sizeof(m_tag_ar.m_size), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	ret = fwrite(m_tag_ar.m_pointer, sizeof(var_u8), m_tag_ar.m_size, _fh);
	if (ret != m_tag_ar.m_size)
	{
		return RET_FALSE;
	}
	//
	ret = fwrite(&m_terms.m_size, sizeof(m_terms.m_size), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	ret = fwrite(m_terms.m_pointer, sizeof(term_info_st), m_terms.m_size, _fh);
	if (ret != m_terms.m_size)
	{
		return RET_FALSE;
	}
	return  RET_SECCEED;
}

var_4 doc_infer::copy(doc_infer* &_dinfer)
{
	m_tm  = _dinfer->m_tm;
	m_id  = _dinfer->m_id;
	m_eid = _dinfer->m_eid;
	// m_buf_ptr is not useful
	m_buf_ptr = _dinfer->m_buf_ptr;
	
	m_tag_cl.m_size = _dinfer->m_tag_cl.m_size; 
	m_tag_cl.m_capacity = _dinfer->m_tag_cl.m_capacity;
	memcpy(m_tag_cl.m_pointer, _dinfer->m_tag_cl.m_pointer, sizeof(var_u8)*_dinfer->m_tag_cl.m_capacity);
	
	m_tag_ar.m_size = _dinfer->m_tag_ar.m_size;
	m_tag_cl.m_capacity = _dinfer->m_tag_ar.m_capacity;
	memcpy(m_tag_ar.m_pointer, _dinfer->m_tag_ar.m_pointer, sizeof(var_u8)*_dinfer->m_tag_ar.m_capacity);

	m_terms.m_size = _dinfer->m_terms.m_size;
	m_terms.m_capacity = _dinfer->m_terms.m_capacity;
	memcpy(m_terms.m_pointer, _dinfer->m_terms.m_pointer, sizeof(term_info_st)*_dinfer->m_terms.m_capacity);
	
	return RET_SECCEED;
}


