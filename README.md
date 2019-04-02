# vision_arduino_android

# 작품 이름: AI 차량 번호 인식 장치

**Google Cloud Vision API**를 사용하여 안드로이드로 차량의 번호표를 촬영하면 블루투스 통신으로 멀리 떨어져 있는 아두이노 LCD에 차량 번호를 전송하는 장치



### Android Studio Code 



#### 1) UI 화면

##### 사진을 불러오는 floating button

![screenshot1](https://user-images.githubusercontent.com/31922389/55395618-d9ce6a80-557c-11e9-9799-cbafbcf98acb.jpg)

#### 2) 실행 사진

![screenshot4](https://user-images.githubusercontent.com/31922389/55395643-e357d280-557c-11e9-9390-c836660715f9.png)
![screenshot5](https://user-images.githubusercontent.com/31922389/55395646-e5219600-557c-11e9-9deb-ab8697399125.png)


```java
 //choose image floatingBtn
        FloatingActionButton fab = findViewById(R.id.fab);
        fab.setOnClickListener(view -> {
            AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
            builder
                    .setMessage(R.string.dialog_select_prompt)
                    .setPositiveButton(R.string.dialog_select_gallery, (dialog, which) -> startGalleryChooser())
                    .setNegativeButton(R.string.dialog_select_camera, (dialog, which) -> startCamera());
            builder.create().show();
        });



        mImageDetails = findViewById(R.id.image_details);
        mMainImage = findViewById(R.id.main_image);
    }
```



먼저 앨범이 그려진 플로팅 버튼은 터치 이벤트가 발생하는  '카메라로 사진찍기' 또는 '앨법에서 사진 불러오기'를 선택하는 버튼이나오고  해당하는 버튼 이벤트에 start함수들이 실행이 되도록 작성되었습니다. 



```java
 public void onClick(View v) {
        int id = v.getId();
        switch (id) {
            case R.id.fab_blue:
                anim();
                break;
            case R.id.fab_connect:
                anim();

                if (bt.getServiceState() == BluetoothState.STATE_CONNECTED) {
                    bt.disconnect();
                } else {
                    Intent intent = new Intent(getApplicationContext(), DeviceList.class);
                    startActivityForResult(intent, BluetoothState.REQUEST_CONNECT_DEVICE);
                }
                break;
            case R.id.fab_send:
                anim();
                setup();
                break;
        }

    }
```

그리고 블루투스 기능을 수행하는 '+' 모양의 플로팅 버튼 역시 블루투스 버튼이 눌리면,  startActivityForResult(intent, BluetoothState.REQUEST_CONNECT_DEVICE); 을 시행하도록 하였고

종기비행기 버튼이 눌리면 , **setup()을 수행해서 ** 연결된 디바이스로 메세지를 보내도록 설정하였습니다. 



이제 본격적으로 기능을 구현하는 코드를 설명하도록 하겠습니다.

위의 코드 중 주요한 역할을 하는 코드는 구글 인공지능 라이브러리를 구현하는 것을 목적으로 작성한 코드와 아두이노와 블루투스로 통신하는 것을 목적으로 작성한 코드입니다. 

#### 2) google cloud vision api 사용

![CloudVisionAPI](C:\Users\Yodi\Desktop\숭실대\2018-2 숭실대\인터랙션 디자인\파이널 과제\new논문\차량 번호판 인식\images\CloudVisionAPI.png)

비전 api같은 경우 구글이 제공하는 오픈 소스를 참고하였기 때문에 밑에 적어놓은 구글 클라우드의 깃허브에 가시면 더욱 자세하고 정확한 정보를 얻으실 수 있습니다. 



```java
public void uploadImage(Uri uri) {
        if (uri != null) {
            try {
                // scale the image to save on bandwidth
                Bitmap bitmap =
                        scaleBitmapDown(
                                MediaStore.Images.Media.getBitmap(getContentResolver(), uri),
                                MAX_DIMENSION);

                callCloudVision(bitmap);
                mMainImage.setImageBitmap(bitmap);

            } catch (IOException e) {
                Log.d(TAG, "Image picking failed because " + e.getMessage());
                Toast.makeText(this, R.string.image_picker_error, Toast.LENGTH_LONG).show();
            }
        } else {
            Log.d(TAG, "Image picker gave us a null image.");
            Toast.makeText(this, R.string.image_picker_error, Toast.LENGTH_LONG).show();
        }
    }
```

위의 함수는 기기에서 이미지를 선택하면 구글 클라우드 비전에 이미지를 업로드하도록 하는 기능을 합니다. 

 callCloudVision(bitmap); 에서 클라우드 비전의 라이브러리에서 이미지를 받고 해당 이미지의 라벨을 리턴하는 역할을 하는 것을 확인하실 수 있습니다. 



```java
private void callCloudVision(final Bitmap bitmap) {
        // Switch text to loading
        mImageDetails.setText(R.string.loading_message);

        // Do the real work in an async task, because we need to use the network anyway
        try {
            AsyncTask<Object, Void, String> labelDetectionTask = new LableDetectionTask(this, prepareAnnotationRequest(bitmap));
            labelDetectionTask.execute();
        } catch (IOException e) {
            Log.d(TAG, "failed to make API request because of other IOException " +
                    e.getMessage());
        }
    }

```

labelDetectionTask.execute(); 은 라벨을 찾아주는 작업을 실행시키도록 합니다. 



```java
 private static String convertResponseToString(BatchAnnotateImagesResponse response) {
        StringBuilder message = new StringBuilder("I found these things:\n\n");

        List<EntityAnnotation> labels = response.getResponses().get(0).getLabelAnnotations();
        if (labels != null) {
            for (EntityAnnotation label : labels) {
                message.append(String.format(Locale.US, "%.3f: %s", label.getScore(), label.getDescription()));
                message.append("\n");
            }
            imgName = labels.get(0).getDescription();
        } else {
            message.append("nothing");
        }

        return message.toString();
    }
```

 List<EntityAnnotation> labels = response.getResponses().get(0).getLabelAnnotations();

**이 코드는 이 프로젝트에서 제일 핵심적인 부분입니다.**  클라우드 api에서 받은 reponse에서 라벨을 받아와서 

라벨배열인 labels에 이미지에서 추측한 label들을 저장합니다.

저는 제일 확률이 높은 제일 첫번째 index에 위치한 라벨 한개만 아두이노로 블루투스 통신을 할 것이기 때문에 

하나만 떼와서 imgName에 넣어줍니다. **imgName = labels.get(0).getDescription();**





#### 3) 블루투스 통신 기능 사용



![screenshot3](https://user-images.githubusercontent.com/31922389/55395638-e18e0f00-557c-11e9-8188-1df9ca23e7ee.png)



```java
 public void onDestroy() {
        super.onDestroy();
        bt.stopService(); //블루투스 중지
    }

    public void onStart() {
        super.onStart();
        if (!bt.isBluetoothEnabled()) { //
            Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(intent, BluetoothState.REQUEST_ENABLE_BT);
        } else {
            if (!bt.isServiceAvailable()) {
                bt.setupService();
                bt.startService(BluetoothState.DEVICE_OTHER); //DEVICE_ANDROID는 안드로이드 기기 끼리

            }
        }


    }
```

먼저 어플리케이션이 시작되면 블루투스를 켜도록 하고, 어플리케이션이 종료되면 블루투스 서비스를 종료하도록 설정을 해주었습니다. 



```java
protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == GALLERY_IMAGE_REQUEST && resultCode == RESULT_OK && data != null) {
            uploadImage(data.getData());
        } else if (requestCode == CAMERA_IMAGE_REQUEST && resultCode == RESULT_OK) {
            Uri photoUri = FileProvider.getUriForFile(this, getApplicationContext().getPackageName() + ".provider", getCameraFile());
            uploadImage(photoUri);
        }
        if (requestCode == BluetoothState.REQUEST_CONNECT_DEVICE) {
            if (resultCode == Activity.RESULT_OK)
                bt.connect(data);
        } else if (requestCode == BluetoothState.REQUEST_ENABLE_BT) {
            if (resultCode == Activity.RESULT_OK) {
                bt.setupService();
                bt.startService(BluetoothState.DEVICE_OTHER);
                //setup();
            } else {
                Toast.makeText(getApplicationContext()
                        , "Bluetooth was not enabled."
                        , Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }

```

그리고 블루투스 플로팅 버튼을 클릭하면 페어링된 장치들의 list를 보여주고 연결하도록 하였고

비행기 버튼을 누르면 연결된 아두이노로 시리얼을 전송하도록 setup()함수를 작성하였습니다. 

아두이노 - 안드로이드 통신을 위해 

bt.setupService();

 **bt.startService(BluetoothState.DEVICE_OTHER);** 이렇게 설정하였습니다. 



```java
 public void setup() {

        bt.send(imgName, true);
        Toast.makeText(getApplicationContext()
                , "메세지 전송 완료", Toast.LENGTH_SHORT).show();
    }
```







### Arduino Code설명


SoftwareSerial btSerial(Tx, Rx); 을 설정하여 블루투스 통신을 하는 시리얼을 설정하였습니다. 

```
if ((btSerial.available())  ) {
    // Wait a bit for the entire message to arrive
    delay(100);
    // Clear the screen
    lcd.clear();

    // Write all characters received with the serial port to the LCD.
    while (btSerial.available() > 0) {
      
      lcd.write(btSerial.read());
    }
  }  
```



블루투스 시리얼 통신이 가능한 상태일때,  어플리케이션에서 연결한 블루투스 통신으로 아두이노는 안드로이드로부터 시리얼 통신을 수신합니다. 

그리고 수신한 시리얼(btSerial)을 그대로 lcd에 출력합도록합 니다. 





#### Google Cloud Vision API 사용



저는 구글에서 제공하는 인공지능 컴퓨터 비전 라이브러리인 'google cloud vison api'를 사용하였습니다.

이 api는 이미지에서 정보 추출,  텍스트 추출, 사람 얼굴인식 등 다양한 기능을 제공하면서 사용 방법도 잘 설명되어 있어서 AI에 관해 배경지식이 없는 개발자들도 사용하기 용이합니다. 

저는 구글 클라우드에서 제공하는 해당 api를 적용한  오픈소스를 기반으로 안드로이드 어플리케이션을 만들었습니다. 


깃허브에서 (https://github.com/GoogleCloudPlatform/cloud-vision) 제가 사용한 API에 관한 자세한 정보를 보실 수 있습니다. 



#### Android-BluetoothSPPLibrary 사용





제가 사용한 블루투스 라이브러리입니다. 이 라이브러리 역시 깃허브(https://github.com/akexorcist/Android-BluetoothSPPLibrary)에서 자세한 정보를 확인하실 수 있습니다. 



```java
annotateImageRequest.setFeatures(new ArrayList<Feature>() {{
                Feature labelDetection = new Feature();
                labelDetection.setType("TEXT_DETECTION");
                labelDetection.setMaxResults(MAX_LABEL_RESULTS);
                add(labelDetection);
            }});
```

```java
 private static String convertResponseToString(BatchAnnotateImagesResponse response) {
        StringBuilder message = new StringBuilder("I found these things:\n\n");

        List<EntityAnnotation> labels = response.getResponses().get(0).getTextAnnotations();
        if (labels != null && labels.size() > 0) {

            EntityAnnotation label = labels.get(0);
            imgName = label.getDescription();

            return label.getDescription();
        }

        return "nothing found";

    }
```

