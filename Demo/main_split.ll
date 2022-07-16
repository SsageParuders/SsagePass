; ModuleID = 'main.ll'
source_filename = "main.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [18 x i8] c"split split_num=7\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [9 x i8] c"main.cpp\00", section "llvm.metadata"
@.str.2 = private unnamed_addr constant [8 x i8] c"Hello~\0A\00", align 1
@.str.3 = private unnamed_addr constant [11 x i8] c"ret is %d\0A\00", align 1
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i32, i8* }] [{ i8*, i8*, i8*, i32, i8* } { i8* bitcast (i32 (i32, i32)* @_Z3addii to i8*), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 5, i8* null }], section "llvm.metadata"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z3addii(i32 noundef %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  br label %.split

.split:                                           ; preds = %2
  %4 = alloca i32, align 4
  br label %.split.split

.split.split:                                     ; preds = %.split
  store i32 %0, i32* %3, align 4
  br label %.split.split.split

.split.split.split:                               ; preds = %.split.split
  store i32 %1, i32* %4, align 4
  br label %.split.split.split.split

.split.split.split.split:                         ; preds = %.split.split.split
  %5 = load i32, i32* %3, align 4
  br label %.split.split.split.split.split

.split.split.split.split.split:                   ; preds = %.split.split.split.split
  %6 = load i32, i32* %4, align 4
  br label %.split.split.split.split.split.split

.split.split.split.split.split.split:             ; preds = %.split.split.split.split.split
  %7 = add nsw i32 %5, %6
  br label %.split.split.split.split.split.split.split

.split.split.split.split.split.split.split:       ; preds = %.split.split.split.split.split.split
  ret i32 %7
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z9say_hellov() #1 {
  %1 = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([8 x i8], [8 x i8]* @.str.2, i64 0, i64 0))
  ret void
}

declare dso_local i32 @printf(i8* noundef, ...) #2

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #3 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  call void @_Z9say_hellov()
  %3 = call noundef i32 @_Z3addii(i32 noundef 10, i32 noundef 20)
  store i32 %3, i32* %2, align 4
  %4 = load i32, i32* %2, align 4
  %5 = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([11 x i8], [11 x i8]* @.str.3, i64 0, i64 0), i32 noundef %4)
  ret i32 0
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress noinline optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 14.0.1"}
