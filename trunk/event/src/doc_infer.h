#ifndef __DOC_INFER_H_
#define __DOC_INFER_H_
#include "UH_Define.h"
#include "word_extractor_API.h"
#include "common_def.h"

class event_container;

class doc_infer
{
public:
	doc_infer();
	
	~doc_infer();

	var_4 deserialize(
		FILE* _fh
		);
	
	var_4 serialize(
		FILE* _fh
		);
	var_4 copy(
		doc_infer*&
		);

public:
	var_u8 m_tm;

	var_u8 m_id;

	var_u8 m_eid;

	var_1* m_buf_ptr;

	static_array<var_u8>	m_tag_cl;
	static_array<var_u8>	m_tag_ar;
	
	static_array<term_info_st>  m_terms;
};

#endif

