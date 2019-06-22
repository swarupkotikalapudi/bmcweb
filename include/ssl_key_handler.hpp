#pragma once
#ifdef BMCWEB_ENABLE_SSL

#include <openssl/bio.h>
#include <openssl/dh.h>
#include <openssl/dsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>

#include <boost/asio/ssl/context.hpp>
#include <random>

namespace ensuressl
{
static void initOpenssl();
static void cleanupOpenssl();
static EVP_PKEY *createRsaKey();
static EVP_PKEY *createEcKey();
static void handleOpensslError();

// Trust chain related errors.`
#define TRUST_CHAIN_ERR(errnum)                                                \
    ((errnum == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) ||                     \
     (errnum == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN) ||                       \
     (errnum == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY) ||               \
     (errnum == X509_V_ERR_CERT_UNTRUSTED) ||                                  \
     (errnum == X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE))

using X509_Ptr = std::unique_ptr<X509, decltype(&::X509_free)>;

inline bool validateCertificate(const std::string &filePath,
                                const X509_Ptr &cert)
{
    // Defining store object as RAW to avoid double free.
    // X509_LOOKUP_free free up store object.
    // Create an empty X509_STORE structure for certificate validation.
    auto x509Store = X509_STORE_new();
    if (!x509Store)
    {
        std::cerr << "Error occured during X509_STORE_new call. ErrorCode: "
                  << ERR_get_error() << std::endl;
        return false;
    }

    // ADD Certificate Lookup method.
    using X509_LOOKUP_Ptr =
        std::unique_ptr<X509_LOOKUP, decltype(&::X509_LOOKUP_free)>;

    X509_LOOKUP_Ptr lookup(X509_STORE_add_lookup(x509Store, X509_LOOKUP_file()),
                           ::X509_LOOKUP_free);
    if (!lookup)
    {
        // Normally lookup cleanup function interanlly does X509Store cleanup
        // Free up the X509Store.
        X509_STORE_free(x509Store);
        std::cerr
            << "Error occured during X509_STORE_add_lookup call. ErrorCode: "
            << ERR_get_error() << std::endl;
        return false;
    }
    // Load Certificate file.
    auto errCode = X509_V_OK;
    errCode = X509_LOOKUP_load_file(lookup.get(), filePath.c_str(),
                                    X509_FILETYPE_PEM);
    if (errCode != 1)
    {
        std::cerr
            << "Error occured during X509_LOOKUP_load_file call. ErroCode: "
            << ERR_get_error() << std::endl;
        return false;
    }

    // Load Certificate file into the X509 structre.
    using X509_STORE_CTX_Ptr =
        std::unique_ptr<X509_STORE_CTX, decltype(&::X509_STORE_CTX_free)>;

    X509_STORE_CTX_Ptr storeCtx(X509_STORE_CTX_new(), ::X509_STORE_CTX_free);
    if (!storeCtx)
    {
        std::cerr << "Error occured during X509_STORE_CTX_new call. ErrorCode: "
                  << ERR_get_error() << std::endl;
        return false;
    }

    errCode = X509_STORE_CTX_init(storeCtx.get(), x509Store, cert.get(), NULL);
    if (errCode != 1)
    {
        std::cerr
            << "Error occured during X509_STORE_CTX_init call. ErrorCode: "
            << ERR_get_error() << std::endl;
        return false;
    }

    // Set time to current time.
    auto locTime = time(nullptr);

    X509_STORE_CTX_set_time(storeCtx.get(), X509_V_FLAG_USE_CHECK_TIME,
                            locTime);

    errCode = X509_verify_cert(storeCtx.get());
    if (errCode == 1)
    {
        errCode = X509_V_OK;
    }
    else if (errCode == 0)
    {
        errCode = X509_STORE_CTX_get_error(storeCtx.get());
        std::cerr << "Certificate verification failed. ErrorCode: " << errCode
                  << std::endl;
    }
    else
    {
        std::cerr << "Error occured during X509_verify_cert call. ErrorCode: "
                  << errCode << std::endl;
        return false;
    }

    // Allow certificate upload, for "certificate is not yet valid" and
    // trust chain related errors.
    if (!((errCode == X509_V_OK) ||
          (errCode == X509_V_ERR_CERT_NOT_YET_VALID) ||
          TRUST_CHAIN_ERR(errCode)))
    {
        if (errCode == X509_V_ERR_CERT_HAS_EXPIRED)
        {
            std::cerr << "Expired Certificate" << std::endl;
        }
        else // Loging general error here.
        {
            std::cerr << "Certificate validation failed" << std::endl;
        }
        return false;
    }

    return true;
}

inline bool verifyOpensslKeyCert(const std::string &filepath)
{
    bool privateKeyValid = false;
    bool certValid = false;

    std::cout << "Checking certs in file " << filepath << "\n";

    FILE *file = fopen(filepath.c_str(), "r");
    if (file != NULL)
    {
        EVP_PKEY *pkey = PEM_read_PrivateKey(file, NULL, NULL, NULL);
        int rc;
        if (pkey != nullptr)
        {
            RSA *rsa = EVP_PKEY_get1_RSA(pkey);
            if (rsa != nullptr)
            {
                std::cout << "Found an RSA key\n";
                if (RSA_check_key(rsa) == 1)
                {
                    privateKeyValid = true;
                }
                else
                {
                    std::cerr << "Key not valid error number "
                              << ERR_get_error() << "\n";
                }
                RSA_free(rsa);
            }
            else
            {
                EC_KEY *ec = EVP_PKEY_get1_EC_KEY(pkey);
                if (ec != nullptr)
                {
                    std::cout << "Found an EC key\n";
                    if (EC_KEY_check_key(ec) == 1)
                    {
                        privateKeyValid = true;
                    }
                    else
                    {
                        std::cerr << "Key not valid error number "
                                  << ERR_get_error() << "\n";
                    }
                    EC_KEY_free(ec);
                }
            }
            EVP_PKEY_free(pkey);
        }
        else
        {
            BMCWEB_LOG_ERROR << "Error in during PEM_read_PrivateKey call";
        }

        fclose(file);

        if (privateKeyValid)
        {
            X509 *x509 = X509_new();
            if (!x509)
            {
                BMCWEB_LOG_ERROR << "Error in during X509_new call";
                return certValid;
            }

            BIO *bioCert = BIO_new_file(filepath.c_str(), "rb");

            if (!bioCert)
            {
                BMCWEB_LOG_ERROR << "Error in during BIO_new_file call";
                X509_free(x509);
                return certValid;
            }

            if (PEM_read_bio_X509(bioCert, &x509, NULL, NULL))
            {
                rc = X509_verify(x509, pkey);
                if (rc != 1)
                {
                    BMCWEB_LOG_ERROR
                        << "Error in verifying private key signature";
                    BIO_free(bioCert);
                    X509_free(x509);
                    return certValid;
                }
                certValid = true;
            }
            else
            {
                BMCWEB_LOG_ERROR << "Error in during PEM_read_bio_X509 call";
            }
            BIO_free(bioCert);
            X509_free(x509);
        }
    }
    return certValid;
}

inline void generateSslCertificate(const std::string &filepath)
{
    FILE *pFile = NULL;
    std::cout << "Generating new keys\n";
    initOpenssl();

    // std::cerr << "Generating RSA key";
    // EVP_PKEY *pRsaPrivKey = create_rsa_key();

    std::cerr << "Generating EC key\n";
    EVP_PKEY *pRsaPrivKey = createEcKey();
    if (pRsaPrivKey != nullptr)
    {
        std::cerr << "Generating x509 Certificate\n";
        // Use this code to directly generate a certificate
        X509 *x509;
        x509 = X509_new();
        if (x509 != nullptr)
        {
            // get a random number from the RNG for the certificate serial
            // number If this is not random, regenerating certs throws broswer
            // errors
            std::random_device rd;
            int serial = rd();

            ASN1_INTEGER_set(X509_get_serialNumber(x509), serial);

            // not before this moment
            X509_gmtime_adj(X509_get_notBefore(x509), 0);
            // Cert is valid for 10 years
            X509_gmtime_adj(X509_get_notAfter(x509),
                            60L * 60L * 24L * 365L * 10L);

            // set the public key to the key we just generated
            X509_set_pubkey(x509, pRsaPrivKey);

            // get the subject name
            X509_NAME *name;
            name = X509_get_subject_name(x509);

            X509_NAME_add_entry_by_txt(
                name, "C", MBSTRING_ASC,
                reinterpret_cast<const unsigned char *>("US"), -1, -1, 0);
            X509_NAME_add_entry_by_txt(
                name, "O", MBSTRING_ASC,
                reinterpret_cast<const unsigned char *>("OpenBMC"), -1, -1, 0);
            X509_NAME_add_entry_by_txt(
                name, "CN", MBSTRING_ASC,
                reinterpret_cast<const unsigned char *>("testhost"), -1, -1, 0);
            // set the CSR options
            X509_set_issuer_name(x509, name);

            // Sign the certificate with our private key
            X509_sign(x509, pRsaPrivKey, EVP_sha256());

            pFile = fopen(filepath.c_str(), "wt");

            if (pFile != nullptr)
            {
                PEM_write_PrivateKey(pFile, pRsaPrivKey, NULL, NULL, 0, 0,
                                     NULL);

                PEM_write_X509(pFile, x509);
                fclose(pFile);
                pFile = NULL;
            }

            X509_free(x509);
        }

        EVP_PKEY_free(pRsaPrivKey);
        pRsaPrivKey = NULL;
    }

    // cleanup_openssl();
}

EVP_PKEY *createRsaKey()
{
    RSA *pRSA = NULL;
#if OPENSSL_VERSION_NUMBER < 0x00908000L
    pRSA = RSA_generate_key(2048, RSA_3, NULL, NULL);
#else
    RSA_generate_key_ex(pRSA, 2048, NULL, NULL);
#endif

    EVP_PKEY *pKey = EVP_PKEY_new();
    if ((pRSA != nullptr) && (pKey != nullptr) &&
        EVP_PKEY_assign_RSA(pKey, pRSA))
    {
        /* pKey owns pRSA from now */
        if (RSA_check_key(pRSA) <= 0)
        {
            fprintf(stderr, "RSA_check_key failed.\n");
            handleOpensslError();
            EVP_PKEY_free(pKey);
            pKey = NULL;
        }
    }
    else
    {
        handleOpensslError();
        if (pRSA != nullptr)
        {
            RSA_free(pRSA);
            pRSA = NULL;
        }
        if (pKey != nullptr)
        {
            EVP_PKEY_free(pKey);
            pKey = NULL;
        }
    }
    return pKey;
}

EVP_PKEY *createEcKey()
{
    EVP_PKEY *pKey = NULL;
    int eccgrp = 0;
    eccgrp = OBJ_txt2nid("prime256v1");

    EC_KEY *myecc = EC_KEY_new_by_curve_name(eccgrp);
    if (myecc != nullptr)
    {
        EC_KEY_set_asn1_flag(myecc, OPENSSL_EC_NAMED_CURVE);
        EC_KEY_generate_key(myecc);
        pKey = EVP_PKEY_new();
        if (pKey != nullptr)
        {
            if (EVP_PKEY_assign_EC_KEY(pKey, myecc))
            {
                /* pKey owns pRSA from now */
                if (EC_KEY_check_key(myecc) <= 0)
                {
                    fprintf(stderr, "EC_check_key failed.\n");
                }
            }
        }
    }
    return pKey;
}

void initOpenssl()
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    RAND_load_file("/dev/urandom", 1024);
#endif
}

void cleanupOpenssl()
{
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    ERR_remove_thread_state(0);
#endif
    EVP_cleanup();
}

void handleOpensslError()
{
    ERR_print_errors_fp(stderr);
}
inline void ensureOpensslKeyPresentAndValid(const std::string &filepath)
{
    bool pemFileValid = false;

    pemFileValid = verifyOpensslKeyCert(filepath);

    if (!pemFileValid)
    {
        std::cerr << "Error in verifying signature, regenerating\n";
        generateSslCertificate(filepath);
    }
}

inline std::shared_ptr<boost::asio::ssl::context>
    getSslContext(const std::string &ssl_pem_file)
{
    std::shared_ptr<boost::asio::ssl::context> mSslContext =
        std::make_shared<boost::asio::ssl::context>(
            boost::asio::ssl::context::tls_server);
    mSslContext->set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::no_sslv3 |
                             boost::asio::ssl::context::single_dh_use |
                             boost::asio::ssl::context::no_tlsv1 |
                             boost::asio::ssl::context::no_tlsv1_1);

    // m_ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
    mSslContext->use_certificate_file(ssl_pem_file,
                                      boost::asio::ssl::context::pem);
    mSslContext->use_private_key_file(ssl_pem_file,
                                      boost::asio::ssl::context::pem);

    // Set up EC curves to auto (boost asio doesn't have a method for this)
    // There is a pull request to add this.  Once this is included in an asio
    // drop, use the right way
    // http://stackoverflow.com/questions/18929049/boost-asio-with-ecdsa-certificate-issue
    if (SSL_CTX_set_ecdh_auto(mSslContext->native_handle(), 1) != 1)
    {
        BMCWEB_LOG_ERROR << "Error setting tmp ecdh list\n";
    }

    std::string mozillaModern = "ECDHE-ECDSA-AES256-GCM-SHA384:"
                                "ECDHE-RSA-AES256-GCM-SHA384:"
                                "ECDHE-ECDSA-CHACHA20-POLY1305:"
                                "ECDHE-RSA-CHACHA20-POLY1305:"
                                "ECDHE-ECDSA-AES128-GCM-SHA256:"
                                "ECDHE-RSA-AES128-GCM-SHA256:"
                                "ECDHE-ECDSA-AES256-SHA384:"
                                "ECDHE-RSA-AES256-SHA384:"
                                "ECDHE-ECDSA-AES128-SHA256:"
                                "ECDHE-RSA-AES128-SHA256";

    if (SSL_CTX_set_cipher_list(mSslContext->native_handle(),
                                mozillaModern.c_str()) != 1)
    {
        BMCWEB_LOG_ERROR << "Error setting cipher list\n";
    }
    return mSslContext;
}
} // namespace ensuressl

#endif
