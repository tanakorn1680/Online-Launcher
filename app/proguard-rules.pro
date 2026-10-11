-keep class com.nvidia.devtech.* { *; }
-keep class com.rockstargames.gtasa.* { *; }
-keep class com.wardrumstudios.utils.* { *; }

-keep class com.gtasan.online.game.* { *; }
-keep class com.gtasan.online.game.ui.* { *; }
-keep class com.gtasan.online.game.ui.widgets.* { *; }
-keep class com.gtasan.online.game.ui.widgets.adapter.* { *; }

-keep class com.gtasan.online.utils.SignatureChecker { *; }

# for minify
-dontwarn javax.servlet.**
-dontwarn org.conscrypt.**
-dontwarn org.bouncycastle.**
-dontwarn org.openjsse.**