/* soapCustomBinding_DumpUploadService3_UploaderProxy.h
   Generated by gSOAP 2.8.17r from DumpUploader.h

Copyright(C) 2000-2013, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
GPL or Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#ifndef soapCustomBinding_DumpUploadService3_UploaderProxy_H
#define soapCustomBinding_DumpUploadService3_UploaderProxy_H
#include "soapH.h"

class SOAP_CMAC CustomBinding_DumpUploadService3_UploaderProxy : public soap
{ public:
    /// Endpoint URL of service 'CustomBinding_DumpUploadService3_UploaderProxy' (change as needed)
    const char *soap_endpoint;
    /// Constructor
    CustomBinding_DumpUploadService3_UploaderProxy();
    /// Construct from another engine state
    CustomBinding_DumpUploadService3_UploaderProxy(const struct soap&);
    /// Constructor with endpoint URL
    CustomBinding_DumpUploadService3_UploaderProxy(const char *url);
    /// Constructor with engine input+output mode control
    CustomBinding_DumpUploadService3_UploaderProxy(soap_mode iomode);
    /// Constructor with URL and input+output mode control
    CustomBinding_DumpUploadService3_UploaderProxy(const char *url, soap_mode iomode);
    /// Constructor with engine input and output mode control
    CustomBinding_DumpUploadService3_UploaderProxy(soap_mode imode, soap_mode omode);
    /// Destructor frees deserialized data
    virtual ~CustomBinding_DumpUploadService3_UploaderProxy();
    /// Initializer used by constructors
    virtual void CustomBinding_DumpUploadService3_UploaderProxy_init(soap_mode imode, soap_mode omode);
    /// Delete all deserialized data (with soap_destroy and soap_end)
    virtual void destroy();
    /// Delete all deserialized data and reset to default
    virtual void reset();
    /// Disables and removes SOAP Header from message
    virtual void soap_noheader();
    /// Get SOAP Header structure (NULL when absent)
    virtual const SOAP_ENV__Header *soap_header();
    /// Get SOAP Fault structure (NULL when absent)
    virtual const SOAP_ENV__Fault *soap_fault();
    /// Get SOAP Fault string (NULL when absent)
    virtual const char *soap_fault_string();
    /// Get SOAP Fault detail as string (NULL when absent)
    virtual const char *soap_fault_detail();
    /// Close connection (normally automatic, except for send_X ops)
    virtual int soap_close_socket();
    /// Force close connection (can kill a thread blocked on IO)
    virtual int soap_force_close_socket();
    /// Print fault
    virtual void soap_print_fault(FILE*);
#ifndef WITH_LEAN
    /// Print fault to stream
#ifndef WITH_COMPAT
    virtual void soap_stream_fault(std::ostream&);
#endif

    /// Put fault into buffer
    virtual char *soap_sprint_fault(char *buf, size_t len);
#endif

    /// Web service operation 'Hello' (returns error code or SOAP_OK)
    virtual int Hello(_ns1__Hello *ns1__Hello, _ns1__HelloResponse *ns1__HelloResponse) { return this->Hello(NULL, NULL, ns1__Hello, ns1__HelloResponse); }
    virtual int Hello(const char *endpoint, const char *soap_action, _ns1__Hello *ns1__Hello, _ns1__HelloResponse *ns1__HelloResponse);

    /// Web service operation 'UploadMiniDump' (returns error code or SOAP_OK)
    virtual int UploadMiniDump(_ns1__UploadMiniDump *ns1__UploadMiniDump, _ns1__UploadMiniDumpResponse *ns1__UploadMiniDumpResponse) { return this->UploadMiniDump(NULL, NULL, ns1__UploadMiniDump, ns1__UploadMiniDumpResponse); }
    virtual int UploadMiniDump(const char *endpoint, const char *soap_action, _ns1__UploadMiniDump *ns1__UploadMiniDump, _ns1__UploadMiniDumpResponse *ns1__UploadMiniDumpResponse);

    /// Web service operation 'UploadFullDump' (returns error code or SOAP_OK)
    virtual int UploadFullDump(_ns1__UploadFullDump *ns1__UploadFullDump, _ns1__UploadFullDumpResponse *ns1__UploadFullDumpResponse) { return this->UploadFullDump(NULL, NULL, ns1__UploadFullDump, ns1__UploadFullDumpResponse); }
    virtual int UploadFullDump(const char *endpoint, const char *soap_action, _ns1__UploadFullDump *ns1__UploadFullDump, _ns1__UploadFullDumpResponse *ns1__UploadFullDumpResponse);

    /// Web service operation 'UploadAdditionalInfo' (returns error code or SOAP_OK)
    virtual int UploadAdditionalInfo(_ns1__UploadAdditionalInfo *ns1__UploadAdditionalInfo, _ns1__UploadAdditionalInfoResponse *ns1__UploadAdditionalInfoResponse) { return this->UploadAdditionalInfo(NULL, NULL, ns1__UploadAdditionalInfo, ns1__UploadAdditionalInfoResponse); }
    virtual int UploadAdditionalInfo(const char *endpoint, const char *soap_action, _ns1__UploadAdditionalInfo *ns1__UploadAdditionalInfo, _ns1__UploadAdditionalInfoResponse *ns1__UploadAdditionalInfoResponse);

    /// Web service operation 'RejectedToSendAdditionalInfo' (returns error code or SOAP_OK)
    virtual int RejectedToSendAdditionalInfo(_ns1__RejectedToSendAdditionalInfo *ns1__RejectedToSendAdditionalInfo, _ns1__RejectedToSendAdditionalInfoResponse *ns1__RejectedToSendAdditionalInfoResponse) { return this->RejectedToSendAdditionalInfo(NULL, NULL, ns1__RejectedToSendAdditionalInfo, ns1__RejectedToSendAdditionalInfoResponse); }
    virtual int RejectedToSendAdditionalInfo(const char *endpoint, const char *soap_action, _ns1__RejectedToSendAdditionalInfo *ns1__RejectedToSendAdditionalInfo, _ns1__RejectedToSendAdditionalInfoResponse *ns1__RejectedToSendAdditionalInfoResponse);

    /// Web service operation 'UploadSymbol' (returns error code or SOAP_OK)
    virtual int UploadSymbol(_ns1__UploadSymbol *ns1__UploadSymbol, _ns1__UploadSymbolResponse *ns1__UploadSymbolResponse) { return this->UploadSymbol(NULL, NULL, ns1__UploadSymbol, ns1__UploadSymbolResponse); }
    virtual int UploadSymbol(const char *endpoint, const char *soap_action, _ns1__UploadSymbol *ns1__UploadSymbol, _ns1__UploadSymbolResponse *ns1__UploadSymbolResponse);
};
#endif
