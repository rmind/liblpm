/*
 * Copyright (c) 2016 Henry Rodrick <henry at holodisc org uk>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

package org.netbsd.liblpm;

import java.io.*;
import java.net.InetAddress;

public class LPM<T> {
	static {
		File temp = null;
		final String name = "org_netbsd_liblpm_LPM.so";
		try {
			InputStream in = LPM.class.getResourceAsStream("/" + name);
			byte[] buffer = new byte[1024];
			int read = -1;
			temp = File.createTempFile(name, "");
			FileOutputStream fos = new FileOutputStream(temp);

			while((read = in.read(buffer)) != -1) {
				fos.write(buffer, 0, read);
			}
			fos.close();
			in.close();

			System.load(temp.getAbsolutePath());
		} catch (IOException e) {
			throw new RuntimeException(e);
		} finally {
			if (temp != null) {
				temp.delete();
			}
		}
	}

	private long lpm;

	private native long init();
	private native void destroy(long lpm);
	private native int insert(long lpm, String cidr, T value);
	private native int insert(long lpm, byte[] address, int prefixLength, T value);
	private native T lookup(long lpm, String addr);
	private native T lookup(long lpm, byte[] address);
	private native int remove(long lpm, String cidr);
	private native int remove(long lpm, byte[] address, int prefixLength);
	private native void clear(long lpm);

	private void validateCIDR(byte[] address, int prefixLength) {
		if (address == null) {
			throw new IllegalArgumentException(
				"address must not be null");
		}
		if (address.length != 4 && address.length != 16) {
			throw new IllegalArgumentException(
				"address length must be 4 or 16 bytes");
		}
		if (prefixLength < 0 || prefixLength > address.length * 8) {
			throw new IllegalArgumentException(
				"prefix length must be >= 0 and <= " +
				(address.length * 8));
		}
	}

	public LPM() {
		lpm = init();
		if (lpm == 0) {
			throw new RuntimeException("Failed to initialize liblpm");
		}
	}

	public boolean insert(String cidr, T value) {
		if (cidr == null) {
			throw new IllegalArgumentException(
				"CIDR must not be null");
		}
		return insert(lpm, cidr, value) == 0;
	}

	public boolean insert(InetAddress inet, int prefixLength, T value) {
		return insert(inet.getAddress(), prefixLength, value);
	}

	public boolean insert(byte[] address, int prefixLength, T value) {
		validateCIDR(address, prefixLength);
		return insert(lpm, address, prefixLength, value) == 0;
	}

	public T lookup(String address) {
		if (address == null) {
			throw new IllegalArgumentException(
				"address must not be null");
		}
		return lookup(lpm, address);
	}

	public T lookup(InetAddress inet) {
		return lookup(inet.getAddress());
	}

	public T lookup(byte[] address) {
		validateCIDR(address, address.length);
		return lookup(lpm, address);
	}

	public boolean remove(String cidr) {
		if (cidr == null) {
			throw new IllegalArgumentException(
				"CIDR must not be null");
		}
		return remove(lpm, cidr) == 0;
	}

	public boolean remove(InetAddress inet, int prefixLength) {
		return remove(inet.getAddress(), prefixLength);
	}

	public boolean remove(byte[] address, int prefixLength) {
		validateCIDR(address, address.length);
		return remove(lpm, address, prefixLength) == 0;
	}

	public void clear() {
		clear(lpm);
	}

	protected void finalize() throws Throwable {
		destroy(lpm);
	}
}
