package ac.kr.kgu.esproject;

import java.util.ArrayList;
import java.util.Random;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

public class ArrayAdderActivity extends Activity {
	static { 
		System.loadLibrary("esterm");
	}
    
	public native int buzzerControl(int value);
	public native int segmentControl(String value);
	public native int dotMatrixControl(String data);
	public native int calculate(int[] arr, int value);
	// Thread Status
	
	private static final int IDLE = 0;
	private static final int PRINT = 1;
	private static final int INTERRUPT = 2;
	private int threadState = IDLE;
	
	private boolean start = false, restart = false;
	private boolean isConfirmed = false, isCorrect = false;
	
	private String value = "";
	private String valueForSegment = "";
	
	BuzzThread buzzThread = new BuzzThread();
	DotThread dotThread = new DotThread();
	SegThread segThread = new SegThread();	
	
	// UI
	Button createBtn, confirmBtn, clearBtn;
	TextView elementList, resultTv, resultOfSummationTv;
	EditText inputValue;
	Spinner spinner;
	
	// Adder
	int[] arguments;
	Context context;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        dotThread.setDaemon(true);
        dotThread.start();
        segThread.setDaemon(true);
        segThread.start();
       // buzzThread.setDaemon(true);
       // buzzThread.start();
        
        // Initialize DotMatrix
        
        
       // UI       
        resultOfSummationTv = (TextView) findViewById(R.id.resultOfSummationTv);
        createBtn = (Button) findViewById(R.id.createBtn);
        confirmBtn = (Button) findViewById(R.id.confirmBtn);
        clearBtn = (Button) findViewById(R.id.clearBtn);
        elementList = (TextView) findViewById(R.id.elementList);
        resultTv = (TextView) findViewById(R.id.resultTv);
        inputValue = (EditText) findViewById(R.id.inputValue);
        spinner = (Spinner) findViewById(R.id.spinner1);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
             R.array.number, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);

        clearBtn.setVisibility(View.GONE);
        resultTv.setVisibility(View.GONE);
        confirmBtn.setVisibility(View.GONE);
        inputValue.setVisibility(View.GONE);
        resultOfSummationTv.setVisibility(View.GONE);
        
        createBtn.setOnClickListener(new Button.OnClickListener() {
			@Override
			public void onClick(View v) {
				Random r = new Random();
				StringBuilder str = new StringBuilder();
				
				int nums = spinner.getSelectedItemPosition() + 1;
				arguments = new int[nums];
				
				for(int i=1; i<=nums; i++){
					arguments[i-1] = r.nextInt(10);
					str.append("element #" + i + " : " + arguments[i-1] + "\n");
				}
				
				elementList.setText(str.toString());	
				
				clearBtn.setVisibility(View.VISIBLE);
		        
		        confirmBtn.setVisibility(View.VISIBLE);
		        inputValue.setVisibility(View.VISIBLE);
		        resultOfSummationTv.setVisibility(View.VISIBLE);
			}
		});
        
        confirmBtn.setOnClickListener(new Button.OnClickListener() {
			@Override
			public void onClick(View v) {
				value = inputValue.getText().toString();
							
				int argValue = Integer.parseInt(value);
				int result = calculate(arguments, argValue);
				
				if(result==argValue) isCorrect = true;
				else isCorrect = false;
				
				if(!isCorrect) {
					value += "-" + result;
					resultTv.setText("That is Wrong.");
				} else {
					resultTv.setText("That is Correct.");
				}
				resultTv.setVisibility(View.VISIBLE);
				
				
				valueForSegment = "";
				
				if(argValue == 0){
					valueForSegment += Getsegmentcode(0);
				} else {
					ArrayList<String> vlist = new ArrayList<String>();
					
					while(argValue > 0){
						vlist.add(Getsegmentcode(argValue % 10));
						argValue = argValue / 10;
					}
					
					for(int i=vlist.size()-1; i>=0; i--){
						valueForSegment += vlist.get(i);
					}
				}
				
				if(!isCorrect){
					if(result != 0){
						valueForSegment += "02";
						
						ArrayList<String> vlist = new ArrayList<String>();
						
						while(result > 0){
							vlist.add(Getsegmentcode(result % 10));
							result = result / 10;
						}
						
						for(int i=vlist.size()-1; i>=0; i--){
							valueForSegment += vlist.get(i);
						}
					} else {
						valueForSegment += "02";
						valueForSegment += Getsegmentcode(0);
					}
				}
				
				isConfirmed = true;
				threadState = PRINT;
			}
		});
        
        clearBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				threadState = IDLE;
				
				isConfirmed = false;
				
				int data = 0;
				buzzerControl(data);
				
				elementList.setText("");
				resultTv.setText("");
				inputValue.setText("");
				
				clearBtn.setVisibility(View.GONE);
		        resultTv.setVisibility(View.GONE);
		        confirmBtn.setVisibility(View.GONE);
		        inputValue.setVisibility(View.GONE);
		        resultOfSummationTv.setVisibility(View.GONE);
			}
		});
    }
    
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			threadState = INTERRUPT;
			buzzThread.interrupt();
			dotThread.interrupt();
			segThread.interrupt();
			
			start = false;
			restart = false;
			
		}
		return super.onKeyDown(keyCode, event);
	}
	
	public String Getsegmentcode (int x)
	{
		String code;
		switch (x) {
			case 0x0 : code = "fc"; break;
			case 0x1 : code = "60"; break;
			case 0x2 : code = "da"; break;
			case 0x3 : code = "f2"; break;
			case 0x4 : code = "66"; break;
			case 0x5 : code = "b6"; break;
			case 0x6 : code = "be"; break;
			case 0x7 : code = "e4"; break;
			case 0x8 : code = "fe"; break;
			case 0x9 : code = "f6"; break;
			
			case 0xa : code = "fa"; break;
			case 0xb : code = "3e"; break;
			case 0xc : code = "1a"; break;
			case 0xd : code = "7a"; break;						
			case 0xe : code = "9e"; break;
			case 0xf : code = "8e"; break;				
			default : code = "00"; break;
		}
		return code;
	}		
	
    class BuzzThread extends Thread{
    	public void run(){
    		while(threadState != INTERRUPT){
    			int data = 0;
    			if(!isConfirmed){
    				data = 0;
    				buzzerControl(data);
    			} else {
    				if(isCorrect){
    					data = 0;
    					while(isConfirmed){
    						try {
    							buzzerControl(data);
        						data = (data == 0) ? 1 : 0;
								sleep(500);
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
    					}
    				} else {
    					data = 1;
    					buzzerControl(data);
    				}
    			}
    		}
    	}
    }
    class DotThread extends Thread{
    	public void run(){
    		while(threadState != INTERRUPT){
	    		if(threadState == IDLE){
	    			try{
	    			  	for(int i=0; i<Constants.dotCircle.length && threadState == IDLE; i++){
		    	    		for(int j=0; j<20; j++){
		    	    			dotMatrixControl(Constants.dotCircle[i]);
		    	    		}
		    	    		sleep(50);
		    	    	}		    	    	
	    			} catch(Exception e){
	    			
	    			}
	        	} else {
	        		if (value.length() > 50) {
	        			
	        		} else {
	        			for(int k=0; k<3 && threadState == PRINT; k++){
		    				for(int i=0; i<20 && threadState == PRINT; i++){
			    	    		dotMatrixControl(Constants.DotCircleAll);
			    	    	}
		    				try{
		    					dotMatrixControl("00000000000000000000");
		    					sleep(100);
		    				} catch(Exception e){
		    					
		    				}
	    				}	
	        			
	        			int i, j, ch;
	        			char buf[] = new char[100];
	
	        			
	        			buf = value.toCharArray();
	
	        			String result = new String();
	        			
	        			result = "00000000000000000000";
	
	        			for (i = 0; i < value.length(); i++) {
	        			
	        				ch = Integer.valueOf(buf[i]);
	
	        			
	        				if (ch < 32 || ch > 126) {
	        					break;
	        				}
	        			
	        				ch -= 0x20;
	
	        				// copy
	        				for (j = 0; j < 5; j++) {
	        					String str = new String();
	        					
	        			
	        					str = Integer.toHexString((Constants.font[ch][j]));
	        					
	        			
	        					if (str.length() < 2)
	        						result += "0";        					
	        			
	        					result += str;
	        				}
	        			
	        				result += "00";
	        			}
	        			
	        			result += "00000000000000000000";
	        	
	        			
	        			// 33
	        			
	        			// print
	        			for (i = 0; i < (result.length() - 18) / 2; i++) {
	        			
	        				for (j = 0; j < 15; j++) {
	        			
	        					if (threadState == IDLE) {
	        						break; 
	        					} else {
	        						dotMatrixControl(result.substring(2 * i,
	        								2 * i + 20));
	        					}
	        				}
	        			}
	        			
	        			//33
	        		}
	        		
	        		for(int k=0; k<3 && threadState == PRINT; k++){
	    				for(int i=0; i<20 && threadState == PRINT; i++){
		    	    		dotMatrixControl(Constants.DotCircleAll);
		    	    	}
	    				try{
	    					dotMatrixControl("00000000000000000000");
	    					sleep(100);
	    				} catch(Exception e){
	    					
	    				}
    				}	
	        		
	        		try{
		        		dotMatrixControl("00000000000000000000");
		        		sleep(100);
	        		} catch(Exception e){
	        			
	        		}
	        	}
    		}
    	}
    }
    class SegThread extends Thread{
    	public void run(){
    		while(threadState != INTERRUPT){
    			if(threadState == IDLE){
    				try{
    					for(int i=0; i<16 && threadState == IDLE; i++){
    						for(int j=0; j<10 && threadState == IDLE; j++){
    							segmentControl(Constants.circleSelect[i]);
    						}
    					}
    					sleep(50);
    				} catch(Exception e){
    				}
    			} else {
    				try{
    					for(int i=0; i<3 && threadState == PRINT; i++){
    						for(int j=0; j<10 && threadState == PRINT; j++){
    							segmentControl(Constants.circle);
    						}
    						sleep(100);
    					}
    					
    				
    					String result = "000000000000";
    					
    					result += valueForSegment;
    					
    					result += "000000000000";
    					
    					for(int i=0; i<result.length()-12 && threadState == PRINT; i+=2){
    						String t = result.substring(i, i+12);
    						
    						for(int j=0; j<10 && threadState == PRINT; j++){
    							segmentControl(t);
    						}
    					}
    					
    					for(int i=0; i<3 && threadState == PRINT; i++){
    						for(int j=0; j<10 && threadState == PRINT; j++){
    							segmentControl(Constants.circle);
    						}
    						sleep(100);
    					}
    					
    					try{
    						segmentControl("000000000000");
    						sleep(100);
    					}catch(Exception e){
    						
    					}
    				} catch(Exception e){
    					
    				}
    			}
    		}
    	}
    }
}