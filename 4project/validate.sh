tar -czvf ypan331_project4.tar.gz protocols/
rm -rf ../valid_4p/project4
rm ../valid_4p/ypan331_project4.tar.gz
cp ypan331_project4.tar.gz ../valid_4p/
cp -r ../4project ../valid_4p/project4

cd ../valid_4p
./verify.sh ypan331_project4.tar.gz

cd -
