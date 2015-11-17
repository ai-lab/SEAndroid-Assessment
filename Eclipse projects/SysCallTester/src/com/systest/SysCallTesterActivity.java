package com.systest;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;

public class SysCallTesterActivity extends Activity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.main);
        Test t = new Test();
        t.execute();
        
        //JavaTest t1 = new JavaTest();
        //t1.execute();
     
        /*Log.d("SysCallTester","avvio del servizio");
        Intent intent = new Intent(this, SysCallService.class);
        startService(intent);
        Log.d("SysCallTester","servizio avviato la prima volta, chiusura activity");*/
        this.finish();
    }
   
    private class JavaTest extends AsyncTask<Void,Void,Integer>{

		@Override
		protected Integer doInBackground(Void... arg0) {
			//create file object
		    RandomAccessFile file = null;
		   
		    int ch;
		    StringBuffer strContent = new StringBuffer("");
		    
		    try
		    {
		      file = new RandomAccessFile("/dev/foo","r");  //non trova nel filesystem!
		      /*
		       * To read bytes from stream use,
		       * int read() method of FileInputStream class.
		       *
		       * This method reads a byte from stream. This method returns next byte of data
		       * from file or -1 if the end of the file is reached.
		       *
		       * Read method throws IOException in case of any IO errors.
		       */
		     
		      while( (ch = file.read()) != -1){
		        strContent.append((char)ch);
		        Log.d("SysTest",strContent.toString());
		      }
		      /*
		       * To close the FileInputStream, use
		       * void close() method of FileInputStream class.
		       *
		       * close method also throws IOException.
		       */
		      file.close();
		    
		    }
		    catch(FileNotFoundException e)
		    {
		    	Log.e("SysTest","File "  +
		                             " could not be found on filesystem");
		    }
		    catch(IOException ioe)
		    {
		    	Log.e("SysTest","Exception while reading the file" + ioe);
		    }

			return null;
		}
    	
    	
    }
    
    
    
    private class Test extends AsyncTask<Void,Void,Integer>{

		@Override
		protected Integer doInBackground(Void... params) {
			Log.d("SysTester","AsyncTask avviato");
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
			return null;
		}
    	
    }
}