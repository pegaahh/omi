/*
**==============================================================================
**
** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
** for license information.
**
**==============================================================================
*/

#ifndef _omi_wsman_wsbuf_h
#define _omi_wsman_wsbuf_h

#include "config.h"
#include <string.h>
#include <common.h>
#include <base/batch.h>
#include <pal/strings.h>
#include <base/messages.h>

#if defined(CONFIG_FAVORSIZE)
# define WSBUF_DISABLE_INLINING
#endif

BEGIN_EXTERNC

/* Error codes needed for compatibility with Windows WinRM */
#define ERROR_WSMAN_SERVICE_STREAM_DISCONNECTED 0x803381DE

/* Allocation limit for WSMAN */
#define WSMAN_ALLOCATION_LIMIT (MAX_ENVELOPE_SIZE * 2)

typedef enum _WSBUF_FAULT_CODE
{
    WSBUF_FAULT_INTERNAL_ERROR,
    WSBUF_FAULT_NOT_SUPPORTED,
    WSBUF_FAULT_NOT_UNDERSTOOD,
    WSBUF_FAULT_DESTINATION_UNREACHABLE,
    WSBUF_FAULT_ACCESS_DENIED,
    WSBUF_FAULT_ENCODING_LIMIT,
    WSBUF_FAULT_INVALID_EXPIRATION_TIME,
    WSBUF_FAULT_UNSUPPORTED_EXPIRATION_TIME,
    WSBUF_FAULT_TIMED_OUT,
    WSBUF_FAULT_INVALID_MESSAGE_INFORMATION_HEADER,
    WSBUF_FAULT_INVALID_HEARTBEAT,
    WSBUF_FAULT_ACTION_NOT_SUPPORTED,
    WSBUF_FAULT_CONNECTION_RETRY_NOT_SUPPORTED,
    WSBUF_FAULT_BOOKMARK_INVALID_FORMAT
}
WSBUF_FAULT_CODE;

typedef struct _WSBuf
{
    Page* page;
    MI_Uint32 position;
}
WSBuf;

MI_Result WSBuf_Init(
    WSBuf* buf,
    MI_Uint32 initialSize);

MI_Result WSBuf_Destroy(
    WSBuf* buf);

MI_Result __WSBuf_AddLit(
    WSBuf* buf,
    const ZChar* str,
    MI_Uint32 size);

MI_Result WSBuf_AddVerbatim(
    WSBuf* buf,
    const void* data,
    MI_Uint32 size);

MI_Result WSBuf_AddStartTag(
    WSBuf* buf,
    const ZChar* tag,
    MI_Uint32 size);

MI_Result WSBuf_AddEndTag(
    WSBuf* buf,
    const ZChar* tag,
    MI_Uint32 size);

MI_Result WSBuf_AddStartTagWithAttrs(
    WSBuf* buf,
    const ZChar* tag,
    MI_Uint32 tagSize,
    const ZChar* attributes,
    MI_Uint32 attributesSize);

MI_Result WSBuf_AddStartTagMustUnderstand(
    WSBuf* buf,
    const ZChar* tag,
    MI_Uint32 tagSize);

#if defined(WSBUF_DISABLE_INLINING)
MI_Result WSBuf_AddLit(
    WSBuf* buf,
    const ZChar* str,
    MI_Uint32 size);

MI_Result WSBuf_AddLit1(
    WSBuf* buf,
    ZChar c1);

MI_Result WSBuf_AddLit2(
    WSBuf* buf,
    ZChar c1,
    ZChar c2);

MI_Result WSBuf_AddLit3(
    WSBuf* buf,
    ZChar c1,
    ZChar c2,
    ZChar c3);

MI_Result WSBuf_AddLit4(
    WSBuf* buf,
    ZChar c1,
    ZChar c2,
    ZChar c3,
    ZChar c4);
MI_Result WSBuf_AddStringNoEncoding(
    WSBuf* buf,
    const ZChar* str);
#else
# include "wsbufinline.h"
#endif

MI_Result WSBuf_AddString(
    WSBuf* buf,
    const ZChar* str);


# define WSBuf_AddCharStringNoEncoding  WSBuf_AddStringNoEncoding
# define WSBuf_AddCharLit               WSBuf_AddLit


MI_Result WSBuf_AddUint32(
    WSBuf* buf,
    MI_Uint32 n);

Page* WSBuf_StealPage(
    WSBuf* buf);

/*
    Converts static/dynamic instance into wsman-xml,
    suitable for concatenating WS-soap xml-response.
    If successful, result buffer page is attached to the batch.
    Parameters:
    instance - instance to convert
    castToClassDecl [opt] - class-decl of output. Used only for 
        deep enumeration with 'base-property-only' flag set.
        If set, this parameter must point to a valid base class.
    batch - batch to borrow memory from
    flags - flags to control instance encoding mode (EPR/Object)
    ptrOut - pointer to result buffer (single string with xml fragment)
    sizeOut - size of result buffer
    Returns:
    OK, FAILED (out of memory)
*/
MI_Result WSBuf_InstanceToBufWithClassName(
    UserAgent userAgent,
    const MI_Instance* instance,
    MI_Boolean (*filterProperty)(const ZChar* name, void* data),
    void* filterPropertyData,
    const MI_ClassDecl* castToClassDecl,
    Batch* batch,
    MI_Uint32 flags,
    const MI_Char *classNameOverride,
    void** ptrOut,
    MI_Uint32* sizeOut);

MI_Result WSBuf_InstanceToBuf(
    UserAgent userAgent,
    const MI_Instance* instance,
    MI_Boolean (*filterProperty)(const ZChar* name, void* data),
    void* filterPropertyData,
    const MI_ClassDecl* castToClassDecl,
    Batch* batch,
    MI_Uint32 flags,
    void** ptrOut,
    MI_Uint32* sizeOut);

MI_Result WSBuf_ClassToBuf(_In_ const MI_Class *classObject, 
                                MI_Uint32 flags,
                                Batch* batch,
                                void** ptrOut,
                                MI_Uint32* sizeOut);

/* Utility */
#define WS_MSG_ID_SIZE      42

MI_Result WSBuf_GenerateMessageID(
    _Pre_writable_size_(WS_MSG_ID_SIZE) ZChar msgID[WS_MSG_ID_SIZE]);

/* Maps CIMM error to the most relevant WS fault;
   Retuns description of CIM error (can be used as fault description) */
WSBUF_FAULT_CODE    WSBuf_CIMErrorToWSFault(
    MI_Uint32       cimErrorCode,
    const ZChar** description );

/* Helper function to create a fault repsonse */
Page* WSBuf_CreateFaultResponsePage(
    WSBUF_FAULT_CODE faultCode,
    const ZChar* notUnderstoodTag,
    const ZChar* requestMessageID,
    const PostResultMsg* message);

Page* WSBuf_CreateReleaseResponsePage(
    const ZChar* requestMessageID);

/* Creates soap header with provided action. 
    Funciotn leaves header open so extra header fields can be added */
MI_Result WSBuf_CreateSoapResponseHeader(
    WSBuf   *buf,
    const ZChar*  action,
    MI_Uint32       actionSize,
    const ZChar*     relatesTo);

#define LIT(str) str,(sizeof(str)/sizeof(str[0])-1)

INLINE ZChar* PageData(Page* page)
{
    return (ZChar*)(page + 1);
}

INLINE ZChar* BufData(WSBuf* buf)
{
    return PageData(buf->page);
}

typedef struct _WsmanClient_Headers
{
    MI_Char *protocol;
    MI_Char *hostname;
    MI_Uint32 port;
    MI_Char *httpUrl;
    MI_Char *resourceUri;
    MI_Char *action;
    MI_Uint32 maxEnvelopeSize;
    MI_Char *locale;
    MI_Char *dataLocale;
    MI_Interval operationTimeout;
    MI_Instance *operationOptions;
#ifndef DISABLE_SHELL
    MI_Char *compressionType;
    MI_Char *sessionId;
#endif
} WsmanClient_Headers;

MI_Result GetMessageRequest(
    WSBuf* buf,                            
    const WsmanClient_Headers *header,
    const GetInstanceReq *request);

MI_Result DeleteMessageRequest(
    WSBuf* buf,                            
    const WsmanClient_Headers *header,
    const DeleteInstanceReq *request);

MI_Result PutMessageRequest(
    WSBuf* buf,                            
    const WsmanClient_Headers *header,
    const ModifyInstanceReq *request);

MI_Result CreateMessageRequest(
    WSBuf* buf,                            
    const WsmanClient_Headers *header,
    const CreateInstanceReq *request);

MI_Result EnumerateMessageRequest(
    WSBuf* buf,                            
    const WsmanClient_Headers *header,
    const EnumerateInstancesReq *request);

MI_Result EnumeratePullRequest(
    WSBuf* buf,
    const WsmanClient_Headers *header,
    const PullReq *request);

MI_Result InvokeMessageRequest(
    WSBuf* buf,                            
    WsmanClient_Headers *header,
    const InvokeReq *request);

/*
MI_Result FindErrorCode(
    WSBUF_FAULT_CODE *faultCode,
    MI_Result *resultCode,
    int action,
    const char *code,
    const char *subCode,
    const MI_Char *reason);
*/
END_EXTERNC

#endif /* _omi_wsman_wsbuf_h */

