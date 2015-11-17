package com.example.writetest;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.webkit.WebView;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        String cls = getPackageName();
        
      //test per vedere se anche java fa così
       try{       
    	//String lib = "sysCallTester.so";
    	String lib = "writer.so";
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
    		Log.e("Writer", "failed to install native library: " + ex);
    	}
       
       WebView myWebView = (WebView) findViewById(R.id.webview);
       myWebView.loadUrl("http://www.example.com");
       myWebView.loadUrl("http://www.yahoo.com");
       myWebView.loadUrl("http://www.bing.com");
       myWebView.loadUrl("http://www.google.com");
       myWebView.loadUrl("http://www.ebay.com");
       myWebView.loadUrl("http://www.youtube.com");
       
       
       //super effective!!!
       //test non nativo
       /*int j = 0;
       while(j <100000){ 
    	   int[] newdata = new int[8092];
    	   try {
    		BufferedWriter br= new BufferedWriter(new FileWriter("/data/data/" + cls + "/" + "temp" + j,true));
			br.append(newdata.toString());
			br.close();
    	   } catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
    	   }    	  
    	   j++;
       }*/
       
       
       //test nativo con file leggibili
       SysTest s = new SysTest();
       int pid = 5; 
        
       BufferedReader br = null;
       
       try
		{
    	    br = new BufferedReader(new FileReader("/data/data/"+cls+"/log.txt"));
			String sCurrentLine;

			while ((sCurrentLine = br.readLine()) != null) {
				System.out.println(sCurrentLine);
				pid = s.copyNative(sCurrentLine,"/data/data/"+cls+"/");
				if(pid >= 0) Log.d("Writer",sCurrentLine + " RIUSCITA");
				else
					Log.e("Writer",sCurrentLine + " FALLITA");
			}

		} catch (IOException e) {
			e.printStackTrace();
		} 
       
       
       
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return true;
    }
    
}
