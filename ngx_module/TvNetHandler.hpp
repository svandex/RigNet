#ifndef _TVNETHANDLER_HPP
#define _TVNETHANDLER_HPP

#include "TvNetConf.hpp"

class TvNetHandler final{
    public:
	typedef TvNetModule this_module;
	typedef TvNetHandler this_type;
    public:
	static ngx_int_t handler(ngx_http_request_t*r)
	    try{
		NgxRequest req(r);

		if(!req.method(NGX_HTTP_GET)){
		    return NGX_HTTP_NOT_ALLOWED;
		}

		req.body().discard();

		auto& cf=this_module::conf().loc(r);

		NgxString msg=cf.msg;
		ngx_str_t teststring=ngx_string("Hi World\n");
		msg=NgxString(teststring);

		//check args
		NgxString args=req->args;

		auto len=msg.size();
		if(!args.empty()){
		    len+=args.size()+1;
		}

		NgxResponse resp(r);

		resp.length(len);
		resp.status(NGX_HTTP_OK);

		if(!args.empty()){
		    resp.send(args);
		    resp.send(",");
		}

		resp.send(msg);

		return resp.eof();
	    }catch(const NgxException& e){
		return e.code();
	    }

};

#endif
