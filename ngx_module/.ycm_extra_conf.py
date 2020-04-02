def Settings(**kwargs):
    return{
            'flags':['-x','c++','-Wall','-Wextra','-Werror',
                '-I','/usr/include/',
                '-I','../include/',
                '-I','./',
                '-I','../../ngx_cpp_dev/ngxpp/',
                '-I','../../ngx_cpp_dev/ngxpp/config/',
                '-I','../../nginx-1.17.7/src/http/',
                '-I','../../nginx-1.17.7/src/core/',
                '-I','../../nginx-1.17.7/src/misc/'
                ]
            }
