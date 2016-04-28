#ifndef __HTTPSTATUS_HPP__
#define __HTTPSTATUS_HPP__


enum class Httpstatus
{

    Continue 						= 100,
    SwitchingProtocols 				= 101,
    Processing 						= 102,
    
    // Success
    OK 								= 200,
    Created 						= 201,
    Accepted 						= 202,
    NonAuthoritativeInformation 	= 203,
    NoContent 						= 204,
    ResetContent 					= 205,
    PartialContent 					= 206,
    MultiStatus 					= 207,
    AlreadyReported 				= 208,
    IMUsed 							= 226,
    
    // Redirections
    MultipleChoices 				= 300,
    MovedPermanently 				= 301,
    Found 							= 302,
    SeeOther 						= 303,
    NotModified 					= 304,
    UseProxy 						= 305,
    SwitchProxy 					= 306,
    TemporaryRedirect 				= 307,
    PermanentRedirect 				= 308,
    
    // Client Errors
    BadRequest 						= 400,
    Unauthorized 					= 401,
    PaymentRequired 				= 402,
    Forbidden 						= 403,
    NotFound 						= 404,
    MethodNotAllowed 				= 405,
    NotAcceptable 					= 406,
    ProxyAuthenticationRequired 	= 407,
    RequestTimeout 					= 408,
    Conflict 						= 409,
    Gone 							= 410,
    LengthRequired 					= 411,
    PreconditionFailed 				= 412,
    RequestEntityTooLarge 			= 413,
    RequestURITooLong 				= 414,
    UnsupportedMediaType 			= 415,
    RequestedRangeNotSatisfiable 	= 416,
    ExpectationFailed 				= 417,
    ImATeapot 						= 418,
    AuthenticationTimeout 			= 419,
    UnprocessableEntity 			= 422,
    Locked 							= 423,
    FailedDependency 				= 424,
    UpgradeRequired 				= 426,
    PreconditionRequired 			= 428,
    TooManyRequests 				= 429,
    RequestHeaderFieldsTooLarge 	= 431,
    LoginTimeout 					= 440,
    NoResponse 						= 444,
    RetryWith 						= 449,
    UnavailableForLegalReasons 		= 451,
    RequestHeaderTooLarge 			= 494,
    CertError 						= 495,
    NoCert 							= 496,
    HTTPToHTTPS 					= 497,
    TokenExpired 					= 498,
    ClientClosedRequest 			= 499,
    
    // Server Errors
    InternalServerError 			= 500,
    NotImplemented 					= 501,
    BadGateway 						= 502,
    ServiceUnavailable 				= 503,
    GatewayTimeout 					= 504,
    HTTPVersionNotSupported 		= 505,
    VariantAlsoNegotiates 			= 506,
    InsufficientStorage 			= 507,
    LoopDetected 					= 508,
    BandwidthLimitExceeded 			= 509,
    NotExtended 					= 510,
    NetworkAuthenticationRequired 	= 511,
    NetworkTimeoutError 			= 599

};


#endif
