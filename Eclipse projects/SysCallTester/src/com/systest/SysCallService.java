package com.systest;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import android.app.Service;
import android.content.Intent;
import android.os.HandlerThread;
import android.os.IBinder;
import android.util.Log;

public class SysCallService extends Service{

	@Override
	public IBinder onBind(Intent arg0) {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	  public void onCreate() {
	    // Start up the thread running the service.  Note that we create a
	    // separate thread because the service normally runs in the process's
	    // main thread, which we don't want to block.  We also make it
	    // background priority so CPU-intensive work will not disrupt our UI.
	    HandlerThread thread = new HandlerThread("SysCallService");
	    thread.start();	    	
	  }
	
	@Override
	  public int onStartCommand(Intent intent, int flags, int startId) {
	    
		Log.d("SysTester","onStart avviata");
        try {
        	String cls = getPackageName();
        	//String lib = "sysCallTester.so";
        	String lib = "netUser.so";
        	String apkLocation = getApplication().getPackageCodePath();
        	//String apkLocation = "/data/app/" + cls + ".apk";
        	String libLocation = "/data/data/" + cls + "/" + lib;
        	ZipFile zip = new ZipFile(apkLocation);
        	ZipEntry zipen = zip.getEntry("assets/" + lib);
        	InputStream is = zip.getInputStream(zipen);
        	OutputStream os = new FileOutputStream(libLocation);
        	byte[] buf = new byte[8092];
        	int n;
        	while ((n = is.read(buf)) > 0) os.write(buf, 0, n);
        	os.close();
        	is.close();
       
        	System.load(libLocation);
        	} catch (Exception ex) {
        	Log.e("SysTester", "failed to install native library: " + ex);
        	}     
        
        SysTest s = new SysTest();
        int pid = 5;
        pid = s.nativeSysTest();
        Log.d("SysTest","esito test:" + pid);
		
		//if whe return here -> restart!
		return START_STICKY;
	  }

}
