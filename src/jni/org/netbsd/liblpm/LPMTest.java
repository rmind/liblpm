/*
 *This file is in the Public Domain.
 */
package org.netbsd.liblpm;

import java.net.InetAddress;

public class LPMTest {
	public static void main(String[] args) throws Exception {
		// Run a few iterations to trigger some GC activity
		for (int n = 0; n < 10; n++) {
			basicTest();
		}
		System.out.println("ok");
	}

	private static void basicTest() throws Exception {
		LPM<String> lpm = new LPM<String>();
		assertEqual(lpm.insert("10.0.0.0/8", "foo"), true);
		assertEqual(lpm.insert("10.10.0.0/16", "bar"), true);
		assertEqual(lpm.insert("10", "bar"), false);
		assertEqual(lpm.insert("2001:db8::1/64", "baz"), true);
		assertEqual(lpm.insert(new byte[]{ 127, 126, 0, 1 }, 15, "raw4"), true);
		assertEqual(lpm.insert(new byte[]{
			127, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0
		}, 66, "raw6"), true);
		assertEqual(lpm.insert(InetAddress.getByName("127.0.0.1"), 32, "lo4"), true);
		assertEqual(lpm.insert(InetAddress.getByName("::1"), 128, "lo6"), true);

		try {
			assertEqual(lpm.insert(new byte[]{127, 0, 1}, 8, "asdf"), true);
			throw new AssertionError("Should throw for illegal address length");
		} catch(IllegalArgumentException e) {
		}

		try {
			assertEqual(lpm.insert(new byte[]{127, 0, 0, 1}, -1, "asdf"), true);
			throw new AssertionError("Should throw for illegal prefix length");
		} catch(IllegalArgumentException e) {
		}

		try {
			assertEqual(lpm.insert(new byte[]{127, 0, 0, 1}, 33, "asdf"), true);
			throw new AssertionError("Should throw for illegal prefix length");
		} catch(IllegalArgumentException e) {
		}

		assertEqual(lpm.lookup("10.0.1.2"), "foo");
		assertEqual(lpm.lookup("10.10.3.4"), "bar");
		assertEqual(lpm.lookup("11.10.3.4"), null);
		assertEqual(lpm.lookup("2001:db8::abcd"), "baz");
		assertEqual(lpm.lookup("127.127.2.3"), "raw4");
		assertEqual(lpm.lookup("127.124.2.3"), null);
		assertEqual(lpm.lookup("7f00:0000:0000:0000:7f00::abcd"), "raw6");
		assertEqual(lpm.lookup("7f00:0000:0000:0000:3f00::abcd"), null);
		assertEqual(lpm.lookup(new byte[]{ 127, 127, 2, 3 }), "raw4");
		assertEqual(lpm.lookup(new byte[]{
			127, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 1
		}), "raw6");
		assertEqual(lpm.lookup(new byte[]{
			127, 0, 0, 0, 0, 0, 0, 0, 63, 0, 0, 0, 0, 0, 0, 1
		}), null);
		assertEqual(lpm.lookup(InetAddress.getByName("127.0.0.1")), "lo4");
		assertEqual(lpm.lookup(InetAddress.getByName("::1")), "lo6");

		assertEqual(lpm.remove("10.10.0.0/16"), true);
		assertEqual(lpm.lookup("10.10.3.4"), "foo");
		assertEqual(lpm.remove(new byte[]{ 127, 126, 0, 1 }, 15), true);
		assertEqual(lpm.lookup(new byte[]{ 127, 127, 2, 3 }), null);
		assertEqual(lpm.remove(new byte[]{
			127, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0
		}, 66), true);
		assertEqual(lpm.lookup(new byte[]{
			127, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 1
		}), null);
		assertEqual(lpm.remove(InetAddress.getByName("127.0.0.1"), 32), true);
		assertEqual(lpm.remove(InetAddress.getByName("::1"), 128), true);
		assertEqual(lpm.lookup(InetAddress.getByName("127.0.0.1")), null);
		assertEqual(lpm.lookup(InetAddress.getByName("::1")), null);

		lpm.clear();
		assertEqual(lpm.lookup("10.10.3.4"), null);

		for (int i = 0; i < 255; i++) {
			for (int j = 0; j < 255; j++) {
				lpm.insert("11.11." + i + "." +
					j + "/32", "foo" + i + "-" + j);
			}
		}
	}

	private static void assertEqual(Object a, Object b) {
		if ((a == null && b != null) || (a != null && !a.equals(b))) {
			throw new AssertionError(
				"Assertion failed: " + a + " != " + b);
		}
	}
}
