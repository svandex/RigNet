#include "precomp.h"

REQUEST_NOTIFICATION_STATUS CRigNet::OnBeginRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider)
{
    UNREFERENCED_PARAMETER(pProvider);

    // Create an HRESULT to receive return values from methods.
    HRESULT hr;

    // Retrieve a pointer to the response.
    IHttpResponse *pHttpResponse = pHttpContext->GetResponse();

    // Test for an error.
    if (pHttpResponse != NULL)
    {
        // Clear the existing response.
        pHttpResponse->Clear();
        // Set the MIME type to plain text.
        pHttpResponse->SetHeader(
            HttpHeaderContentType, "text/plain",
            (USHORT)strlen("text/plain"), TRUE);

        // Create a string with the response.
        PCSTR pszBuffer = "Hello World!";
        // Create a data chunk.
        HTTP_DATA_CHUNK dataChunk;
        // Set the chunk to a chunk in memory.
        dataChunk.DataChunkType = HttpDataChunkFromMemory;
        // Buffer for bytes written of data chunk.
        DWORD cbSent;

        // Set the chunk to the buffer.
        dataChunk.FromMemory.pBuffer =
            (PVOID)pszBuffer;
        // Set the chunk size to the buffer size.
        dataChunk.FromMemory.BufferLength =
            (USHORT)strlen(pszBuffer);
        // Insert the data chunk into the response.
        hr = pHttpResponse->WriteEntityChunks(
            &dataChunk, 1, FALSE, TRUE, &cbSent);
        auto path = svandex::tools::GetCurrentPath();
        char *userppath[256];
        size_t userppath_num;
        _dupenv_s(userppath, &userppath_num, "userprofile");

        LOG(INFO) << path << std::endl;
        LOG(INFO) << userppath[0] << std::endl;
        LOG(INFO) << "Responded." << std::endl;

        // Test for an error.
        if (FAILED(hr))
        {
            // Set the HTTP status.
            pHttpResponse->SetStatus(500, "Server Error", 0, hr);
        }

        // End additional processing.
        return RQ_NOTIFICATION_FINISH_REQUEST;
    }

    // Return processing to the pipeline.
    return RQ_NOTIFICATION_CONTINUE;
}