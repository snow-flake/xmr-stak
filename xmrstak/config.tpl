R"===(
/*
 * pool_address    - Pool address should be in the form "pool.supportxmr.com:3333". Only stratum pools are supported.
 * wallet_address  - Your wallet, or pool login.
 * pool_password   - Can be empty in most cases or "x".
 * use_tls         - This option will make us connect using Transport Layer Security.
 * tls_fingerprint - Server's SHA256 fingerprint. If this string is non-empty then we will check the server's cert against it.
 * pool_weight     - Pool weight is a number telling the miner how important the pool is. Miner will mine mostly at the pool 
 *                   with the highest weight, unless the pool fails. Weight must be an integer larger than 0.
 *
 * We feature pools up to 1MH/s. For a more complete list see M5M400's pool list at www.moneropools.com
 */
"pool_list" :
[
POOLCONF],

/*
 * Manual hardware AES override
 *
 * Some VMs don't report AES capability correctly. You can set this value to true to enforce hardware AES or
 * to false to force disable AES or null to let the miner decide if AES is used.
 *
 * WARNING: setting this to true on a CPU that doesn't support hardware AES will crash the miner.
 */
"aes_override" : null,

)==="
		
