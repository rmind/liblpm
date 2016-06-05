/*
 *This file is in the Public Domain.
 */
package org.netbsd.liblpm;

public class LPMTest {
	public static void main(String[] args) {
		// Run a few iterations to trigger some GC activity
		for (int n = 0; n < 10; n++) {
			LPM<String> lpm = new LPM<String>();
			assertEqual(lpm.insert("10.0.0.0/8", "foo"), true);
			assertEqual(lpm.insert("10.10.0.0/16", "bar"), true);
			assertEqual(lpm.insert("10", "bar"), false);
			assertEqual(lpm.insert("2001:db8::1/64", "baz"), true);

			assertEqual(lpm.lookup("10.0.1.2"), "foo");
			assertEqual(lpm.lookup("10.10.3.4"), "bar");
			assertEqual(lpm.lookup("11.10.3.4"), null);
			assertEqual(lpm.lookup("2001:db8::abcd"), "baz");

			assertEqual(lpm.remove("10.10.0.0/16"), true);
			assertEqual(lpm.lookup("10.10.3.4"), "foo");

			lpm.clear();
			assertEqual(lpm.lookup("10.10.3.4"), null);

			for (int i = 0; i < 255; i++) {
				for (int j = 0; j < 255; j++) {
					lpm.insert("11.11." + i + "." +
						j + "/32", "foo" + i + "-" + j);
				}
			}
		}
		System.out.println("ok");
	}

	private static void assertEqual(Object a, Object b) {
		if ((a == null && b != null) || (a != null && !a.equals(b))) {
			throw new AssertionError(
				"Assertion failed: " + a + " != " + b);
		}
	}
}
