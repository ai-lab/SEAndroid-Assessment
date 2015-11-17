package com.example.writetest;

public class SysTest {
		
	public SysTest() {
		super();
		// TODO Auto-generated constructor stub
	}
 
	public native int nativeSysTest(String path);
	
	public native int copyNative(String sPath, String dPath);

}
