This repo contains some info and scripts that let you re-create addresses
that Electrum uses, without actually having Electrum. It can also help
you generate a custom wallet seed for Electrum.

It's mostly targeted at hackers who want to easily understand what's going
on behind the hood.

# Passphrase to seed
Converting any passphrase to a valid Electrum wallet seed (new format) by
appending an appropriate word:

```
echo -n $PASSPHRASE | ./electrumize-seed
```

which is a faster version of:

```
for t in `seq 0 2047`
do
	if echo -n $PASSPHRASE $t | openssl sha512 -hmac "Seed version" | grep -q '= 01'
	then
		echo $PASSPHRASE $dict[$t]
		break
	fi
done
```

This currently only supports standard (P2PKH) and multisig P2SH wallets
(version 0x01).

*WARNING*: This doesn't do any passphrase normalization, like Electrum does.
Ensure your passphrase doesn't have double-spaces, and do whatever UTF8
conversions Electrum does (I'm not sure what it is exactly). Basically
a bunch of ASCII-alphanumeric words separated by single spaces should work
fine.

[Read more about this](http://electrum.readthedocs.io/en/latest/seedphrase.html)

# Seed to BIP32
Converting an Electrum wallet seed (new format) to BIP32 entropy material:

```
echo -n $SEED | ./pbkdf2 'electrum'$PASSPHRASE
```

where `$PASSPHRASE` is the passphrase used (or an empty string if it wasn't).
Sorry to leak it via the cmdline.

# BIP32 to keys
`bx` is the [libbitcoin-explorer](https://github.com/libbitcoin/libbitcoin-explorer) tool.

To get from the BIP32 entropy material (obtained above) to *public* bitcoin
keys (addresses) in a standard wallet:

```
bx hd-new $ENTROPY | bx hd-public -i $CHANGE | bx hd-public -i $ID | bx hd-to-ec | bx ec-to-address
```
where `$CHANGE` is 0 for receiving addresses and 1 for change addresses.
`$ID`s start at 0.

The provided `ele_seed21st` script combines both above steps into one
(with $ID = 0).

To get the corresponding private keys in [WIF](https://en.bitcoin.it/wiki/Wallet_import_format):
```
bx hd-new $ENTROPY | bx hd-private -i $CHANGE | bx hd-private -i $ID | bx hd-to-ec | bx ec-to-wif
```

You can also try this on the [BIP32 Generator](http://bip32.org/). You need
to select Custom Derivation Path - Electrum uses `m/$CHANGE/$ID`, e.g. `m/0/0`.

# Multisig
To get from the entropy obtained in step *Seed to BIP32* to the `xpub` key that
you can give to cosigners:
```
bx hd-new $ENTROPY | bx hd-to-public
```

To get the EC public key for a given address:

```
bx hd-new $ENTROPY | bx hd-public -i $CHANGE | bx hd-public -i $ID | bx hd-to-ec
```

or the same EC public key generated just from the `xpub` address (yours
or cosigner's):

```
bx hd-public -i $CHANGE $XPUB | bx hd-public -i $ID | bx hd-to-ec
```

Once you get all 3 such EC addresses, you can convert them to an address
in the regular Bitcoin format (one that can be paid to):

```
bx script-to-address "$M [$EC1] [$EC2] [$EC3] $N checkmultisig"
```

where `$M` and `$N` are the parameters of the multisig (`$M`-out-of-`$N`),
and `$EC1` through `$EC3` (or however big `$N` is) are the EC addresses
of different cosigners.

*WARNING:* order of the addresses seems to be important. At a quick glance
they're sorted in ascending order. *TODO:* confirm this.

*TODO:* figure out how to partially-sign things
