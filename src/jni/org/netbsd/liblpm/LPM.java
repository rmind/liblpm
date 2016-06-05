/*
 * Copyright (c) 2016 Henry Rodrick <henry at holodisc org uk>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

package org.netbsd.liblpm;

import java.io.*;

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
	private native T lookup(long lpm, String addr);
	private native int remove(long lpm, String cidr);
	private native void clear(long lpm);

	public LPM() {
		lpm = init();
		if (lpm == 0) {
			throw new RuntimeException("Failed to initialize liblpm");
		}
	}

	public boolean insert(String cidr, T value) {
		return insert(lpm, cidr, value) == 0;
	}

	public T lookup(String addr) {
		return lookup(lpm, addr);
	}

	public boolean remove(String cidr) {
		return remove(lpm, cidr) == 0;
	}

	public void clear() {
		clear(lpm);
	}

	protected void finalize() throws Throwable {
		destroy(lpm);
	}
}
